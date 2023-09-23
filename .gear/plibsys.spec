%define soname 0

Name: plibsys
Version: 0.0.4
Release: alt1

Summary: Highly portable C system library
License: MIT
Group: Development/C

Url: https://github.com/saprykin/plibsys
Source: %name-%version.tar

BuildPreReq: gcc-c++ cmake rpm-macros-cmake doxygen

%description
plibsys is a cross-platform system C library with some helpful routines.
It has zero third-party dependencies and uses only native system calls.

plibsys provides:

Platform independent data types
Threads
Mutexes
Condition variables
Read-write locks
System-wide semaphores
System-wide shared memory
Optimized spinlock
Atomic operations
Socket support (UDP, TCP, SCTP) with IPv4 and IPv6
Hash functions: MD5, SHA-1, SHA-2, SHA-3, GOST (R 34.11-94)
Binary trees: BST, red-black, AVL
INI file parser
High resolution time profiler
Files and directories
Shared library loading
Useful routines for linked lists, strings, hash tables
Macros for CPU architecture, OS and compiler detection
Macros for variable arguments

%package -n lib%name%soname
Summary: %summary
Group: System/Libraries

%description -n lib%name%soname
Highly portable C system library: threads and synchronization primitives,
sockets (TCP, UDP, SCTP), IPv4 and IPv6, IPC, hash functions (MD5, SHA-1,
SHA-2, SHA-3, GOST), binary trees (RB, AVL) and more.
Native code performance.

%package -n lib%name-devel
Summary: %summary
Group: Development/C

%description -n lib%name-devel
Highly portable C system library: threads and synchronization primitives,
sockets (TCP, UDP, SCTP), IPv4 and IPv6, IPC, hash functions (MD5, SHA-1,
SHA-2, SHA-3, GOST), binary trees (RB, AVL) and more.
Native code performance.

This package provides headers to build software using %name.

%package -n %name-doc
Summary: %name docs
Group: Development/Documentation

%description -n %name-doc
HTML API documentation for the plibsys library

%prep
%setup

%build
%cmake_insource \
	-DCMAKE_INSTALL_LIBDIR=%_lib \
	-DPLIBSYS_BUILD_STATIC=OFF \
	-DPLIBSYS_TESTS=OFF \
	-DPLIBSYS_BUILD_DOC=ON \
	-DCMAKE_SKIP_RPATH=ON \
	#
%make_build

%install
%makeinstall_std
rm -rf %buildroot%_defaultdocdir/*

%files -n lib%name%soname
%doc AUTHORS COPYING NEWS README.md
%_libdir/*.so.*

%files -n lib%name-devel
%_includedir/*
%_libdir/*.so

%files -n %name-doc
%doc doc/html/*

%changelog
* Sat May 26 2018 Alexander Saprykin <saprykin.spb@gmail.com> 0.0.4-alt1
- Update to version 0.0.4
* Tue Sep 19 2017 Michael Shigorin <mike@altlinux.org> 0.0.3-alt1
- Initial build

