/**
 * @file   ConfigFileParser.h
 * @author Sebastien Varrette <Sebastien.Varrette@imag.fr>
 * @date   Fri Dec  9 2005  
 *
 * Copyright (c) 2005 Sebastien Varrette  (http://www-id.imag.fr/~svarrett/)
 *
 * @brief  This library provides a mechanism to parse and update a configuration 
 *         file. It follows my experiments with confloader of Nicolas Bernard 
 *         (http://www.lafraze.net/nbernard/) and is partly inspired by the work of
 *         Jeff Schiller(CodeDread) http://www.codedread.com/code/ConfigParser/docs/
 *         (Yet, this work appears to be only available for windows users) and Rick 
 *         Wagner (http://www-personal.engin.umich.edu/~wagnerr/ConfigFile.html).
 *         This last work was really interesting yet I decided to improve this code 
 *         for a better management of file writing and entry access. 
 *         So finally I wrote my own library using my own programming style :-)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Sebastien Varrette                               \n
 * <Sebastien.Varrette@imag.fr>                     \n
 * University of Luxembourg / ID-IMAG Laboratory    \n
 * 162-A avenue de la faïencerie                    \n
 * L-1511 Luxembourg, LUXEMBOURG                    \n
 */
 /********************************************************************************/
#ifndef __CONFIGFILEPARSER_H
#define __CONFIGFILEPARSER_H


#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/// These functions are used to faciliate conversion to/from string from/to a type T
template<class T> T      FromStringTo(const string & s);
template<class T> string ToStringFrom(const T & v);

/**
 * This is the main class representing a configuration file and making it possible 
 * to read/write configuration entries. 
 */
class ConfigFileParser {
    // Write or read configuration
    friend ostream& operator<<(ostream& os, const ConfigFileParser & f);
    friend istream& operator>>(istream& is, ConfigFileParser & f);

private:
    string _filename;		  /**< name of the configuration file           */
    string _delimiter;		  /**< separator between key and value          */
    string _comment;		  /**< character(s) used to introduce a comment */
    map<string,string> _contentMap; /**< the mapping (key, value)               */

    /// Redefinition of iterators types to faciliate navigation in _contents
    typedef map<string,string>::iterator       mapi;
    typedef map<string,string>::const_iterator mapci;
    
    static void _serialize(string & s);/**< Remove leading and trailing whitespace */
    /// return 'true' if s contains a comment (which is returned in cs)
    bool _containsComment(const string & s, string & cs) const ;
    /// extract comment from the string line s
    string _getCommentFrom(const string & s) const; 
    /// extract key, value and comment from the string line s
    bool _checkKeyPresence(const string & s, string & key, string & value, 
			   string & cs, bool & valueWithGuillemet) const;
    
public:
    /// Default constructor without a file
    ConfigFileParser() : _delimiter("="), _comment("#") {}
    /// Main constructor with a file name.
    ConfigFileParser(const string & filename, const string & delimiter = "=", 
		     const string & comment = "#");
    ~ConfigFileParser() {}

    /*** Accessors ***/
    string getFilename()          const { return _filename;  }
    string getDelimiter()         const { return _delimiter; }
    string getCommentIntroducer() const { return _comment;   }
    /// Check if the key exists.
    bool exist(const string & key) const 
          { return _contentMap.find(key) != _contentMap.end(); }
    /// Check if filename exist.
    bool exist() const { return ifstream(_filename.c_str()).is_open(); }
    // Get value associated to a given key 
    template<class T> T getValue(const string & key )                        const;  
    template<class T> T getValue(const string & key, const T & defaultValue) const;
    template<class T> bool getValueInto(T & var, const string & key)         const;
    template<class T> bool getValueInto(T & var, const string & key, 
					const T& defaultValue) const;
    // Return a string value associated to key (eventually throw KeyNotFound)
    string operator[](const string & key) const;
    string & operator[](const string & key);
    /// Return iterators to the beginning and the end of _contentMap
    mapci begin() const { return _contentMap.begin(); }
    mapci end()   const { return _contentMap.end();   }

    /*** Mutators ***/
    void setFilename(const string & f)          { _filename = f;  }
    void setDelimiter(const string & d)         { _delimiter = d; }
    void setCommentIntroducer(const string & c) { _comment = c;   }
    /// Add a new keys/values
    template<class T> void add(string key, const T & value);
    /// remove a key
    void remove(const string& key);
    /// Set value associated to a given key
    template<class T> void setValue(const string & key, const T & value);
    /// Update the configuration file with the content of the map
    void saveCfgFile(const string & filename = "");

    /*** Print methods ***/
    void printMapContent(ostream & os = cout);         /**< Print map content */
    void printCurrentFileContent(ostream & os = cout); /**< output file content */

    /// Exception types when the file is not found
    struct FileNotFound {
	string filename;
	FileNotFound(const string & f = "") : filename(f) {}      };
    /// Exception types when the file can't be created
    struct FileNotCreated {
	string filename;
	FileNotCreated(const string & f = "") : filename(f) {}    };
    /// Exception types when a key is not found
    struct KeyNotFound {
	string key;
	KeyNotFound(const string & k = "") : key(k) {}            };
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
// Template functions have to be declared in the same file

/** 
 * FromStringTo convert a string to a value of type T 
 * Example : int i = FromStringTo<int>(s)
 * @param s string to convert 
 * @return  value of type T corresponding to the conversion of s to this type
 */
template<class T> 
T FromStringTo(const std::string & s) {
    istringstream is(s);
    T t;
    is >> t;
    return t;
}
/**
 * Special version for string i.e when nothing has to be done 
 */
template<> inline string FromStringTo<string>(const string & v) { return v; }
/**
 * Special version for bool values 
 * "false", "F", "no", "n", "0" are interpreted as false
 * "true", "T", "yes", "y", "1", "-1", or anything else are interpreted as true
 */
template<> inline bool FromStringTo<bool>(const string & v) { 
    // same capitalisation for v (faciliate comparisons
    string sup = v;
    for(string::iterator it = sup.begin(); it != sup.end(); it++) *it = toupper(*it);
    if( sup==string("FALSE") || sup==string("F") || sup==string("NO") || 
	sup==string("N") || sup==string("0") || sup==string("NONE") )
	return false;
    else return true;
}


/** 
 * ToStringFrom convert any value v of type T to string 
 * Exemple: string s = ToStringFrom(14)
 * @param v value to convert
 * @return  a string corresponding to v
 */
template<class T> 
string ToStringFrom(const T & v) {
    ostringstream s;
    s << v;
    return s.str();
}


/** 
 * Get the value corresponding to key. Throw KeyNotFound when the key doesn't exist
 * @param key    the name of the key 
 * @return       the value corresponding
 */
template<class T> 
T ConfigFileParser::getValue(const string & key ) const {
    if ( !exist(key) ) throw KeyNotFound(key); 
    return FromStringTo<T>( _contentMap.find(key)->second );
}

/** 
 * Get the value corresponding to key. 
 * @param key            the name of the key 
 * @param defaultValue   default value to return if the key is not found
 * @return               the value corresponding to the key, or defaultValue if the 
 *                       key doesn't exist.
 */
template<class T> 
T ConfigFileParser::getValue(const string & key, const T & defaultValue) const {
    if ( !exist(key) ) return defaultValue;
    return FromStringTo<T>( _contentMap.find(key)->second );
}

/** 
 * Get the value corresponding to key and store it in var
 * @param var    the variable where the key value will be stored (if the key exists)
 * @param key    the name of the key 
 * @return       'true' if key is found, 'false' otherwise and in that case, var is 
 *               unchanged.
 */
template<class T> 
bool ConfigFileParser::getValueInto(T & var, const string & key) const {
    if ( !exist(key) ) return false;
    var = FromStringTo<T>( _contentMap.find(key)->second );
    return true;
}

/** 
 * Get the value corresponding to key and store it in var
 * @param var    the variable where the key value will be stored (if the key exists)
 * @param key    the name of the key 
 * @param defaultValue   default value to return if the key is not found
 * @return       'true' if key is found, 'false' otherwise and in that case, var is 
 *               unchanged.
 */
template<class T> 
bool ConfigFileParser::getValueInto(T & var, const string & key, 
				    const T& defaultValue) const {
    bool ok = exist(key);
    if (ok) var =  FromStringTo<T>( _contentMap.find(key)->second );
    else    var = defaultValue;
    return ok;
}

/** 
 * Set the value associated to key (throw KeyNotFound when the key doesn't exist)
 * @param key      the name of the key 
 * @param value    the associated value
 */
template<class T> 
void ConfigFileParser::setValue(const string & key, const T & value) {
    if ( !exist(key) ) throw KeyNotFound(key); 
    _contentMap[key] = ToStringFrom<T>(value);
}
template<> 
inline void ConfigFileParser::setValue<bool>(const string & key, const bool & value) {
    if ( !exist(key) ) throw KeyNotFound(key); 
    _contentMap[key] = (value ? "yes":"no");
}



/** 
 * Add a new map (key, value) to _contentMap. If the key already exist, its value is
 * overwritted.
 * @param key      the name of the key 
 * @param value    the associated value
 */
template<class T>
void ConfigFileParser::add(string key, const T & value) {
    string v = ToStringFrom<T>(value);
    _serialize(key); 
    _serialize(v);
    _contentMap[key] = v;
}
template<>
inline void ConfigFileParser::add(string key, const bool & value) {
    string v = (value ? "yes":"no");
    _serialize(key); 
    _contentMap[key] = v;
}

#endif  // __CONFIGFILEPARSER_H



