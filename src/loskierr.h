#ifndef loskierr_h
#define loskierr_h


typedef enum loski_ErrorCode {
	LOSKI_ERRNONE = 0, /* no error */
	LOSKI_ERRCLOSED, /* resource closed */
	LOSKI_ERRINUSE, /* resource in use */
	LOSKI_ERRNOTFOUND, /* resource not found */
	LOSKI_ERRUNAVAILABLE, /* resource unavailable */
	LOSKI_ERRUNREACHABLE, /* resource unreachable */
	LOSKI_ERRNORESOURCES, /* resource exhausted */
	LOSKI_ERRTOOMUCH, /* resource overflow */
	LOSKI_ERRNOMEMORY, /* insufficient memory */
	LOSKI_ERRUNFULFILLED, /* operation unfulfilled */
	LOSKI_ERRABORTED, /* operation forcibly aborted */
	LOSKI_ERRREFUSED, /* operation refused */
	LOSKI_ERRDENIED, /* operation denied */
	LOSKI_ERRTIMEOUT, /* operation timeout */
	LOSKI_ERRSYSTEMRESET, /* system reset */
	LOSKI_ERRSYSTEMDOWN, /* system down */
	LOSKI_ERRSYSTEMFAIL, /* system error */
	/* unrecoverable */
	LOSKI_ERRINVALID, /* invalid operation */
	LOSKI_ERRUNSUPPORTED, /* unsupported operation */
	LOSKI_ERRUNEXPECTED, /* unexpected error */
	LOSKI_ERRUNSPECIFIED /* unspecified error */
} loski_ErrorCode;


#endif
