/* Copyright (C) 2008 Pere Negre
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "gtk-gui.h"


/* WARNING: Remember to run valgrind */
int main(int argc,char *argv[])
{
    // Thread safety, see: http://bugzilla.gnome.org/show_bug.cgi?id=524128
    g_thread_init(0);
    gdk_threads_init();

    // From the intltool docs
    //setlocale (LC_ALL, "");
    // URL: http://www.gnome.org/~malcolm/i18n/i18n-header.html
    // URL: http://www.gtkmm.org/docs/gtkmm-2.4/docs/tutorial/html/ch24s02.html
    bindtextdomain(GETTEXT_PACKAGE, WHITEBOARDLOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    MainGtkWindow main_window(argc, argv);
    return main_window.run();
}
