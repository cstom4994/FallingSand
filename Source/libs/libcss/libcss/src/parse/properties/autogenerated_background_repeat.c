/*
 * This file was generated by LibCSS gen_parser 
 * 
 * Generated from:
 *
 * background_repeat:CSS_PROP_BACKGROUND_REPEAT IDENT:( INHERIT: NO_REPEAT:0,BACKGROUND_REPEAT_NO_REPEAT REPEAT_X:0,BACKGROUND_REPEAT_REPEAT_X REPEAT_Y:0,BACKGROUND_REPEAT_REPEAT_Y REPEAT:0,BACKGROUND_REPEAT_REPEAT IDENT:)
 * 
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2010 The NetSurf Browser Project.
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse background_repeat
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse_background_repeat(css_language *c,
		const parserutils_vector *vector, int *ctx,
		css_style *result)
{
	int orig_ctx = *ctx;
	css_error error;
	const css_token *token;
	bool match;

	token = parserutils_vector_iterate(vector, ctx);
	if ((token == NULL) || ((token->type != CSS_TOKEN_IDENT))) {
		*ctx = orig_ctx;
		return CSS_INVALID;
	}

	if ((lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match) == lwc_error_ok && match)) {
			error = css_stylesheet_style_inherit(result, CSS_PROP_BACKGROUND_REPEAT);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[NO_REPEAT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_REPEAT, 0,BACKGROUND_REPEAT_NO_REPEAT);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[REPEAT_X], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_REPEAT, 0,BACKGROUND_REPEAT_REPEAT_X);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[REPEAT_Y], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_REPEAT, 0,BACKGROUND_REPEAT_REPEAT_Y);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[REPEAT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_REPEAT, 0,BACKGROUND_REPEAT_REPEAT);
	} else {
		error = CSS_INVALID;
	}

	if (error != CSS_OK)
		*ctx = orig_ctx;
	
	return error;
}

