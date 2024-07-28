namespace dotnet;

using System;
using System.Runtime.InteropServices;

class Program
{
    enum HandlerStrategy
    {
        InvokePrev,
        InvokeDfl,
    }

    [DllImport("sighandler", EntryPoint = "install_signal_handler")]
    static extern int install_signal_handler(HandlerStrategy strategy);

    [DllImport("sighandler", EntryPoint = "native_crash")]
    static extern void native_crash();

    static void Main(string[] args)
    {
        var doNativeCrash = args is ["native-crash"];

        if (install_signal_handler(HandlerStrategy.InvokePrev) == 1)
        {
            Console.WriteLine("Failed to install signal handler");
            return;
        }

        if (doNativeCrash)
        {
            native_crash();
        }
        else
        {
            var s = default(string);
            var c = s.Length;
        }
    }
}