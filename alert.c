#include "alert.h"
#include "os.h"
#include <assert.h>

#if defined(IS_MACOSX)
	#include <CoreFoundation/CoreFoundation.h>
#elif defined(USE_X11)
	#include <stdio.h> /* For fputs() */
	#include <stdlib.h> /* For exit() */
	#include <sys/wait.h> /* For wait() */
	#include <unistd.h> /* For fork() */
	#include <sys/types.h> /* For pid_t */
	#include "snprintf.h" /* For asprintf() */
#endif

#if defined(USE_X11)

enum {
	TASK_SUCCESS = 0,
	FORK_FAILED = -1,
	EXEC_FAILED = -2
};

/*
 * Unfortunately, X has no standard method of displaying alerts, so instead we
 * have to rely on the shell command "xmessage" (or nicer-looking equivalents).
 *
 * The return value and arguments are the same as those from to runTask()
 * (see below).
 */
static int xmessage(char *argv[], int *exit_status);

#elif defined(IS_MACOSX)
	#define CFStringCreateWithUTF8String(string) \
		((string) == NULL ? NULL : CFStringCreateWithCString(NULL, \
		                                                     string, \
		                                                     kCFStringEncodingUTF8))
#endif

int showAlert(const char *title, const char *msg, const char *defaultButton,
              const char *cancelButton)
{
#if defined(IS_MACOSX)
	CFStringRef alertHeader = CFStringCreateWithUTF8String(title);
	CFStringRef alertMessage = CFStringCreateWithUTF8String(msg);
	CFStringRef defaultButtonTitle = CFStringCreateWithUTF8String(defaultButton);
	CFStringRef cancelButtonTitle = CFStringCreateWithUTF8String(cancelButton);
	CFOptionFlags responseFlags;
	SInt32 err = CFUserNotificationDisplayAlert(0.0,
	                                            kCFUserNotificationNoteAlertLevel,
	                                            NULL,
	                                            NULL,
	                                            NULL,
	                                            alertHeader,
	                                            alertMessage,
	                                            defaultButtonTitle,
	                                            cancelButtonTitle,
	                                            NULL,
	                                            &responseFlags);
	if (alertHeader != NULL) CFRelease(alertHeader);
	if (alertMessage != NULL) CFRelease(alertMessage);
	if (defaultButtonTitle != NULL) CFRelease(defaultButtonTitle);
	if (cancelButtonTitle != NULL) CFRelease(cancelButtonTitle);

	if (err != 0) return -1;
	return (responseFlags == kCFUserNotificationDefaultResponse) ? 0 : 1;
#elif defined(USE_X11)
	/* Note that args[0] is set by the xmessage() function. */
	const char *args[10] = {NULL, msg, "-title", title, "-center"};
	int response, ret;
	char *buttonList = NULL; /* To be free()'d. */

	if (defaultButton == NULL) defaultButton = "OK";

	if (cancelButton == NULL) {
		asprintf(&buttonList, "%s:2", defaultButton);
	} else {
		asprintf(&buttonList, "%s:2,%s:3", defaultButton, cancelButton);
	}

	if (buttonList == NULL) return -1; /* asprintf() failed. */
	args[5] = "-buttons";
	args[6] = buttonList;
	args[7] = "-default";
	args[8] = defaultButton;
	args[9] = NULL;

	ret = xmessage((char **)args, &response);
	if (buttonList != NULL) {
		free(buttonList);
		buttonList = NULL;
	}

	if (ret != TASK_SUCCESS) {
		if (ret == EXEC_FAILED) {
			fputs("xmessage or equivalent not found.\n", stderr);
		}
		return -1;
	}

	return (response == 2) ? 0 : 1;
#else
	/* TODO: Display custom buttons instead of the pre-defined "OK"
	 * and "Cancel". */
	int response = MessageBox(NULL, msg, title,
	                          (cancelButton == NULL) ? MB_OK : MB_OKCANCEL);
	return (response == IDOK) ? 0 : 1;
#endif
}

#if defined(USE_X11)

/*
 * Attempts to run the given task synchronously with the given arguments.
 *
 * If |exit_status| is non-NULL and the task ran successfully, |exit_status| is
 * set to the exit code of the task on return.
 *
 * Returns -1 if process could not be forked, -2 if the task could not be run,
 * or 0 if the task was ran successfully.
 */
static int runTask(const char *taskname, char * const argv[], int *exit_status);

static int xmessage(char *argv[], int *exit_status)
{
	static const char * const MSG_PROGS[] = {"gmessage", "gxmessage",
	                                         "kmessage", "xmessage"};
	static int PREV_MSG_INDEX = -1;
	#define MSG_PROGS_LEN (sizeof(MSG_PROGS) / sizeof(MSG_PROGS[0]))

	char *prog = NULL;
	int ret;

	/* Save some fork()'ing and attempt to use last program if possible. */
	if (PREV_MSG_INDEX >= 0) {
		assert(PREV_MSG_INDEX < MSG_PROGS_LEN);

		prog = argv[0] = (char *)MSG_PROGS[PREV_MSG_INDEX];
		ret = runTask(prog, argv, exit_status);
	} else {
		/* Otherwise, try running each xmessage alternative until one works or
		 * we run out of options. */
		size_t i;
		for (i = 0; i < MSG_PROGS_LEN; ++i) {
			prog = argv[0] = (char *)MSG_PROGS[i];
			ret = runTask(prog, argv, exit_status);
			if (ret != EXEC_FAILED) break;
		}

		if (ret == TASK_SUCCESS) PREV_MSG_INDEX = i;
	}

	return ret;
}

static int runTask(const char *taskname, char * const argv[], int *exit_status)
{
	pid_t pid;
	int status;

	switch (pid = fork()) {
		case -1: /* Failed to fork */
			perror("fork");
			return FORK_FAILED; /* Failed to fork. */
		case 0: /* Child process */
			execvp(taskname, argv);
			exit(42); /* Failed to run task. */
		default: /* Parent process */
			wait(&status); /* Block execution until finished. */

			if (!WIFEXITED(status) || (status = WEXITSTATUS(status)) == 42) {
				return EXEC_FAILED; /* Task failed to run. */
			}
			if (exit_status != NULL) *exit_status = status;
			return TASK_SUCCESS; /* Success! */
	}
}

#endif
