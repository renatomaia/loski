#ifndef luaosi_errors_h
#define luaosi_errors_h


typedef enum losi_ErrorCode {
	LOSI_ERRNONE = 0, /* no error */
	LOSI_ERRCLOSED, /* resource closed */
	LOSI_ERRINUSE, /* resource in use */
	LOSI_ERRNOTFOUND, /* resource not found */
	LOSI_ERRUNAVAILABLE, /* resource unavailable */
	LOSI_ERRUNREACHABLE, /* resource unreachable */
	LOSI_ERRNORESOURCES, /* resource exhausted */
	LOSI_ERRTOOMUCH, /* resource overflow */
	LOSI_ERRNOMEMORY, /* insufficient memory */
	LOSI_ERRUNFULFILLED, /* operation unfulfilled */
	LOSI_ERRABORTED, /* operation forcibly aborted */
	LOSI_ERRREFUSED, /* operation refused */
	LOSI_ERRDENIED, /* operation denied */
	LOSI_ERRTIMEOUT, /* operation timeout */
	LOSI_ERRSYSTEMRESET, /* system reset */
	LOSI_ERRSYSTEMDOWN, /* system down */
	LOSI_ERRSYSTEMFAIL, /* system error */
	/* unrecoverable */
	LOSI_ERRINVALID, /* invalid operation */
	LOSI_ERRUNSUPPORTED, /* unsupported operation */
	LOSI_ERRUNEXPECTED, /* unexpected error */
	LOSI_ERRUNSPECIFIED /* unspecified error */
} losi_ErrorCode;


#endif
