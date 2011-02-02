
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_SLASH = 258,
     TOK_OPEN = 259,
     TOK_CLOSE = 260,
     TOK_STRING = 261,
     TOK_INTEGER = 262,
     TOK_FLOAT = 263,
     TOK_BOOLEAN = 264,
     TOK_IPV6TYPE = 265,
     TOK_DEBUGLEVEL = 266,
     TOK_IPVERSION = 267,
     TOK_HNA4 = 268,
     TOK_HNA6 = 269,
     TOK_PLUGIN = 270,
     TOK_INTERFACE_DEFAULTS = 271,
     TOK_INTERFACE = 272,
     TOK_NOINT = 273,
     TOK_TOS = 274,
     TOK_OLSRPORT = 275,
     TOK_RTPROTO = 276,
     TOK_RTTABLE = 277,
     TOK_RTTABLE_DEFAULT = 278,
     TOK_RTTABLE_TUNNEL = 279,
     TOK_RTTABLE_PRIORITY = 280,
     TOK_RTTABLE_DEFAULTOLSR_PRIORITY = 281,
     TOK_RTTABLE_TUNNEL_PRIORITY = 282,
     TOK_RTTABLE_DEFAULT_PRIORITY = 283,
     TOK_WILLINGNESS = 284,
     TOK_IPCCON = 285,
     TOK_FIBMETRIC = 286,
     TOK_USEHYST = 287,
     TOK_HYSTSCALE = 288,
     TOK_HYSTUPPER = 289,
     TOK_HYSTLOWER = 290,
     TOK_POLLRATE = 291,
     TOK_NICCHGSPOLLRT = 292,
     TOK_TCREDUNDANCY = 293,
     TOK_MPRCOVERAGE = 294,
     TOK_LQ_LEVEL = 295,
     TOK_LQ_FISH = 296,
     TOK_LQ_AGING = 297,
     TOK_LQ_PLUGIN = 298,
     TOK_LQ_NAT_THRESH = 299,
     TOK_LQ_MULT = 300,
     TOK_CLEAR_SCREEN = 301,
     TOK_PLPARAM = 302,
     TOK_MIN_TC_VTIME = 303,
     TOK_LOCK_FILE = 304,
     TOK_USE_NIIT = 305,
     TOK_SMART_GW = 306,
     TOK_SMART_GW_ALLOW_NAT = 307,
     TOK_SMART_GW_UPLINK = 308,
     TOK_SMART_GW_UPLINK_NAT = 309,
     TOK_SMART_GW_SPEED = 310,
     TOK_SMART_GW_PREFIX = 311,
     TOK_SRC_IP_ROUTES = 312,
     TOK_MAIN_IP = 313,
     TOK_HOSTLABEL = 314,
     TOK_NETLABEL = 315,
     TOK_MAXIPC = 316,
     TOK_IFMODE = 317,
     TOK_IPV4MULTICAST = 318,
     TOK_IP4BROADCAST = 319,
     TOK_IPV4BROADCAST = 320,
     TOK_IPV6MULTICAST = 321,
     TOK_IPV4SRC = 322,
     TOK_IPV6SRC = 323,
     TOK_IFWEIGHT = 324,
     TOK_HELLOINT = 325,
     TOK_HELLOVAL = 326,
     TOK_TCINT = 327,
     TOK_TCVAL = 328,
     TOK_MIDINT = 329,
     TOK_MIDVAL = 330,
     TOK_HNAINT = 331,
     TOK_HNAVAL = 332,
     TOK_AUTODETCHG = 333,
     TOK_IPV4_ADDR = 334,
     TOK_IPV6_ADDR = 335,
     TOK_DEFAULT = 336,
     TOK_AUTO = 337,
     TOK_NONE = 338,
     TOK_COMMENT = 339
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


