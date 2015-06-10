XwinTox and Toxaemia
==============

Overview
--------------

*XwinTox* (and its counterpart, *Toxaemia*,) is a different Tox client: 
a radical one. 

It maintains a true division between interface and logic by placing libToxCore 
communication in one binary, processing messages received from the libToxCore 
binary in one thread of another binary, and implementing the user interface in 
another thread of that latter binary. *XwinTox* is the user interface side; 
*Toxaemia* is the service that communicates with Tox.

The process-level seperation of libToxCore communication from communique
processing and user interface implementation has not yet been actualised. In its
stead the libToxCore communication occurs within a seperate thread.

XwinTox and Toxaemia are free software; XwinTox is available under the Peer 
Production Licence (see CPYRIGHT) and Toxaemia (in this context referring to all
code found within the toxaemia_src folder) may be used under the provisions of
the Peer Production Licence or the GPL version 3.0 or later (see FSF.org).


Building
--------------

*Note*: **XwinTox natively uses the SunshinePro Toolchain.
However, as SunshinePro D-Make has a POSIX featureset with a few
GNU extensions I implemented, it is possible to build with *GNU Make*.**

Modify ```Makefile.cfg``` as appropriate to your platform and run GNU make.
If you want Clang's static analyser to run (this is not available if you use GNU
CC) you can do so by running ```make real```

Toxaemia Protocol
--------------

Copied from toxaemia.x.

```c
struct ToxConnectArgs {
	int BootstrapPort;
	string BootstrapAddress<>;
	string BootstrapKey<>;
	string Name<>;
	string Status<>;
};

program TOXAEMIA_PROG {
	version TOXAEMIA_VERS1 {
		int ToxConnect(ToxConnectArgs) = 1;
	} = 1;
} = 0x22159817;
```


