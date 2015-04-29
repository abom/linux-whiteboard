# Building Whiteboard #

First, check out the sources:

```
svn checkout http://linux-whiteboard.googlecode.com/svn/trunk/ whiteboard
```

The new build system is scons-based. Simply issue these commands:

```
scons PREFIX=/usr DESTDIR=/
scons install
```