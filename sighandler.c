#include<stddef.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

static struct sigaction g_sigaction;
static struct sigaction g_previous_handler = { 0 };

static void invoke_previous_handler(int signum, siginfo_t *info, void *user_context)
{
    struct sigaction *handler = &g_previous_handler;
    if (handler->sa_handler == SIG_DFL) {
        raise(signum);
    } else if (handler->sa_flags & SA_SIGINFO) {
        handler->sa_sigaction(signum, info, user_context);
    } else if (handler->sa_handler != SIG_IGN) {
        void (*func)(int) = handler->sa_handler;
        func(signum);
    }
}

static void handle_sigsegv(int signum, siginfo_t *info, void *user_context)
{
    printf("SIGSEGV received \xF0\x9F\x91\x8B\n");

    // the previous handler is the donet runtime handler 
    // converting this signal into a NullReferenceException
    invoke_previous_handler(signum, info, user_context);
}

int install_signal_handler()
{
    printf("Store dotnet runtime handler for signal chain\n");
    // save the dotnet runtime handler
    if (sigaction(SIGSEGV, NULL, &g_previous_handler) == -1) {
    	printf("Failed to retrieve dotnet runtime handler\n");
        return 1;
    }

    // install our own SIGSEGV handler
    printf("Installing our SIGSEGV handler on top\n");
    sigemptyset(&g_sigaction.sa_mask);
    g_sigaction.sa_sigaction = handle_sigsegv;
    g_sigaction.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &g_sigaction, NULL);

    return 0;
}
