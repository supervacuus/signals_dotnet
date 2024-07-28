#include<stddef.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

enum handler_strategy
{
    INVOKE_PREV,
    INVOKE_DFL,
};

static struct sigaction g_sigaction;
static struct sigaction g_previous_handler = { 0 };
static int g_strategy = INVOKE_PREV;

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
    fprintf(stderr, "SIGSEGV received \xF0\x9F\x91\x8B\n");

    if (g_strategy == INVOKE_PREV) {
        // the previous handler is the dotnet runtime handler
        // converting this signal into a NullReferenceException
        invoke_previous_handler(signum, info, user_context);

        // We won't see this if the dotnet handler converts
        // the signal to a managed code exception
        fprintf(stderr, "After dotnet runtime handler\n");
    } else {
        // pass on to the default handler to terminate given SIGSEGV
        struct sigaction sa;
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGSEGV, &sa, NULL);
        raise(signum);
    }
}

void native_crash(void)
{
    *((int*)10) = 100;
}

int install_signal_handler(int strategy)
{
    printf("Store dotnet runtime handler for signal chain\n");
    // save the dotnet runtime handler
    if (sigaction(SIGSEGV, NULL, &g_previous_handler) == -1) {
    	printf("Failed to retrieve dotnet runtime handler\n");
        return 1;
    }

    g_strategy = strategy;

    // install our own SIGSEGV handler
    printf("Installing our SIGSEGV handler on top\n");
    sigemptyset(&g_sigaction.sa_mask);
    g_sigaction.sa_sigaction = handle_sigsegv;
    g_sigaction.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &g_sigaction, NULL);

    return 0;
}
