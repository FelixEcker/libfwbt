libfwbt bindings for fpc

-- ABOUT --

These bindings are a manual translation of the original header file, alongside
some wrapper functions to make working with libfwbt more fpc-like.
All functions which directly link to a libfwbt function are prefixed with `__extern_`,
but should probably be avoided.

Details about the API Version & co can be found within the unit (libfwbt.pas).

-- TESTING --

These bindings come with a simple test file (test.pas) which can also be used as an example for
how to use them. The test can be simply compiled with `fpc test.pas`
