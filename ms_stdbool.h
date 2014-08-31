#pragma once
#if !defined(MS_STDBOOL_H) && \
	(!defined(__bool_true_false_are_defined) || __bool_true_false_are_defined)
#define MS_STDBOOL_H

#ifndef _MSC_VER
	#error "Use this header only with Microsoft Visual C++ compilers!"
#endif /* _MSC_VER */

#define __bool_true_false_are_defined 1

#ifndef __cplusplus

	#if defined(true) || defined(false) || defined(bool)
		#error "Boolean type already defined"
	#endif

	enum {
		false = 0,
		true = 1
	};

	typedef unsigned char bool;

#endif /* !__cplusplus */

#endif /* MS_STDBOOL_H */
