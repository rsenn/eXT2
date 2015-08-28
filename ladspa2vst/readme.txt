ladspa2vst_bridge v0.3 - ladspa 2 vst plugins bridge class
Copyright (C) 2007 kRAkEn/gORe (www.anticore.org)

This is experimental software. Is based around ladspa plugins
specification and steinberg vst specifications.

To compile your ladspa plugins under a vst environment you will
need to download the complete vst sdk from steinberg.
Then define you LADSPA_INIT function (typically _init) and
LADSPA_FINI function (typically _fini) and what is the global
descriptor for the plugin (usually g_psDescriptor). Then include
you ladspa .h/.c file and compile. This would produce a shared
library loadable from any linux native vst hosts.

There is a lot of room to improve:

- fix toggle parameters that u can't set actually
- handle parameters in a better way (a class will do)
- scale port with their ranges accordingly
- make the bridge more ladpsa plugins aware (cmt, amb, blop...)
