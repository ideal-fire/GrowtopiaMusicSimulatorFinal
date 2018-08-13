/*
Copyright © 1994–2017 Lua.org, PUC-Rio.
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef NATHANSDLLUA
#define NATHANSDLLUA
	#include <lua.h>

	#ifndef OVERWRITEOLDMACRO
		// If you want to replace the luaL_dofile macro with the SDL version.
		#define OVERWRITEOLDMACRO 1
	#endif

	typedef struct LoadF {
	  int n;  /* number of pre-read characters */
	  CROSSFILE *f;  /* file being read */
	  char buff[BUFSIZ];  /* area for reading file */
	} LoadF;

	static const char* getF_SDL (lua_State *L, void *ud, size_t *size) {
		LoadF *lf = (LoadF *)ud;
		(void)L;  /* not used */
		if (lf->n > 0) {  /* are there pre-read characters to be read? */
			*size = lf->n;  /* return them (chars already in buffer) */
			lf->n = 0;  /* no more pre-read characters */
		}
		else {  /* read a block from file */
			/* 'fread' can return > 0 *and* set the EOF flag. If next call to
			   'getF' called 'fread', it might still wait for user input.
			   The next check avoids this problem. */
			if (crossfeof(lf->f)){
				return NULL;
			}
			*size = crossfread(lf->buff, 1, sizeof(lf->buff), lf->f);  /* read block */
		}
		//lf->buff[*size]='\0';
		return lf->buff;
	}
	
	static int skipBOM (LoadF *lf) {
		const char *p = "\xEF\xBB\xBF";  /* UTF-8 BOM mark */
		int c;
		lf->n = 0;
		do {
			c = crossgetc(lf->f);
			if (c == EOF || c != *(const unsigned char *)p++) return c;
			lf->buff[lf->n++] = c;  /* to be read by the parser */
		} while (*p != '\0');
		lf->n = 0;  /* prefix matched; discard it */
		return crossgetc(lf->f);  /* return next character */
	}
	
	/*
	** reads the first character of file 'f' and skips an optional BOM mark
	** in its beginning plus its first line if it starts with '#'. Returns
	** true if it skipped the first line.  In any case, '*cp' has the
	** first "valid" character of the file (after the optional BOM and
	** a first-line comment).
	*/
	static int skipcomment (LoadF *lf, int *cp) {
		int c = *cp = skipBOM(lf);
		if (c == '#') {  /* first line is a comment (Unix exec. file)? */
			do {  /* skip first line */
			  c = crossgetc(lf->f);
			} while (c != EOF && c != '\n');
			*cp = crossgetc(lf->f);  /* skip end-of-line, if present */
			return 1;  /* there was a comment */
		}
		else return 0;  /* no comment */
	}
	
	int luaL_loadfilexSDL (lua_State *L, const char *filename, const char *mode) {
		LoadF lf;
		int status;
		int c;
		int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
		lua_pushfstring(L, "@%s", filename);
		lf.f=crossfopen(filename,"r");

		if (lf.f==NULL){
			printf("not exist.");
			printf("%s\n",filename);
			return 7;
		}
		if (skipcomment(&lf, &c))  /* read initial portion IT IS CONFIRMED THAT THIS COMMAND WORKS */
			lf.buff[lf.n++] = '\n';  /* add line to correct line numbers */
		if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!\nCANNOT REOPEN IN BINARY MODE!\n!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
		if (c != EOF)
			lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
		status = lua_load(L, getF_SDL, &lf, lua_tostring(L, -1), mode); // Next to edit is this?
		crossfclose(lf.f);  /* close file (even in case of errors) */
		lua_remove(L, fnameindex);
		return status;
	}
	//http://lua-users.org/lists/lua-l/2007-06/msg00335.html
	
	// Use SDL file read and write functions to open a lua file and run it.
	// I think this macro works because loadfile returns 0 if everything is good, so it'll evaluate the pcall because it's looking for something to evaluate to anything but 0. If loadfile isn't all good, it won't return 0 and therefor won't pcall.
	// HOWEVER, THERE IS A PROBLEM. Because I'm using the || thingie, it'll return 1 if either of them is not 1. So, if one returns 7, this macro will still return 1. I'm not the one who designed this, don't look at me. However, it's probably better because you wouldn't be able to tell pcall and loadfile errors apart.
	#if OVERWRITEOLDMACRO == 0
		#define luaL_dofileSDL(luaState, filename) (luaL_loadfilexSDL(luaState, filename, NULL) || lua_pcall(luaState, 0, LUA_MULTRET, 0))
	#elif OVERWRITEOLDMACRO == 1
		#undef luaL_dofile
		#define luaL_dofile(luaState, filename) (luaL_loadfilexSDL(luaState, filename, NULL) || lua_pcall(luaState, 0, LUA_MULTRET, 0))
	#else
		#error HEY, INVALID OVERWRITEOLDMACRO VALUE, YOU IDIOT!
	#endif
	//int LuaL_DoFileSDL(lua_State* luaState, char* filename){
	//	int _val = luaL_loadfilexSDL(luaState, filename, NULL);
	//	if (_val!=0){
	//		return _val;
	//	}
	//	_val = lua_pcall(luaState, 0, LUA_MULTRET, 0);
	//	return _val;
	//}
#endif