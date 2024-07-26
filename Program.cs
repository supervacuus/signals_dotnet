namespace dotnet;

using System;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("sighandler", EntryPoint="install_signal_handler")]
    static extern int install_signal_handler();
    static void Main(string[] args)
    {
	    install_signal_handler();
	    var s = default(string);
	    var c = s.Length;
    }
}
