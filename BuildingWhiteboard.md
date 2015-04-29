To check out the CPP branch of Linux Whiteboard -which the dependencies below mostly apply to- issue the following command:
```
svn checkout http://linux-whiteboard.googlecode.com/svn/branches/cpp/ linux-whiteboard-read-only
```

To build whiteboard, you'll need the usual toolchain (gcc, g++ and make - these are available in the **build-essential** package) and the following libraries. The names of the libraries are taken from Ubuntu's database. If you're using a different Linux distro, please change them according to yours:

  * libtool
  * libcwiid1-dev
  * libgtkmm-2.4-dev
  * libglademm-2.4-dev
  * libxtst-dev
  * libcairomm-1.0-dev
  * libsigc++-2.0-dev
  * libgnome2-dev
  * intltool
  * libbluetooth-dev

If you have checked out whiteboard from our SVN repository, you probably need those too:
  * autoconf
  * automake

Having gotten those installed, navigate to the program's root directory and issue the following commands:
```
./autogen.sh
make
sudo make install
```


---


For **Ubuntu Gutsy Gibbon** users, you need to add **CXXFLAGS="-DCOMPATIBILITY\_GUTSY"** to autogen.sh like this:
```
./autogen.sh CXXFLAGS="-DCOMPATIBILITY_GUTSY"
```


---


To install to a different directory, so that you don't need root permission when installing, add **--prefix=**

&lt;path-to-desired-install-dir&gt;

**like this:
```
./autogen --prefix=~/whiteboard
```**

After 'make install', whiteboard will be installed to the directory 'whiteboard' in your home directory.


---


To build a Debian/Ubuntu package (for Ubuntu Hardy Heron only, one needs to modify the 'debian' directory to account for version differences in other distros):
  * Install **fakeroot** if you haven't got it already.
  * Issue these commands:
```
./autogen.sh
dpkg-buildpackage -us -uc -b -rfakeroot
```