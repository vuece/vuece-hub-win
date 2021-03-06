#include "stdafx.h"
#include "VueceHubVersion.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

//
// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers
// by using the '*' as shown below:

//TODO This must be the same as the version defined in VueceVersion.h to avoid confusion
//Currently we need to manually modify this to release an update
//Also need to keep consistent with the version files for inno setup
//[assembly:AssemblyVersionAttribute("@MAJORx@.@MINORx@.@BUILDx@.@REVx@")];
//#ifdef CMD_LINE_BUILD
//[assembly:AssemblyVersionAttribute("1.0.1.207")];
//#else
//[assembly:AssemblyVersionAttribute("0.0.0.3")];
//#endif

//[assembly:AssemblyVersionAttribute("1.0.1.207")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

[assembly:SecurityPermission(SecurityAction::RequestMinimum, UnmanagedCode = true)];
