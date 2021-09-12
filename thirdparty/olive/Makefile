#
# 1.      COPYRIGHT NOTICE
#
#       Copyright 1994 Synopsys, Inc. All Rights Reserved.
#
#       Olive is owned by Synopsys and protected by copyright law.  It is
# not public domain software.  Synopsys is making Olive available to you free
# of charge for research and educational purposes only.  You may not sell
# Olive or any product derived from it of which Olive is a significant part
# of the value of the product.  You may use the output of Olive in products
# provided you charge for only those components that are entirely your own
# and you acknowledge the use of Olive clearly in all product documentation
# and distribution media.  You must also request that bug reports on your
# product be communicated to you.
#
#
# 2.      DISCLAIMER
#
#
#         THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND WITHOUT
# WARRANTY OF ANY KIND.  SYNOPSYS AND ITS LICENSORS EXPRESSLY DISCLAIM ALL
# WARRANTIES, EXPRESS, IMPLIED, STATUTORY OR OTHERWISE REGARDING THE SOFTWARE
# AND DOCUMENTATION, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, OR ARISING FROM A
# COURSE OF DEALING OR USAGE OF TRADE.  UNDER NO CIRCUMSTANCES, INCLUDING
# NEGLIGENCE, WILL SYNOPSYS OR ITS LICENSORS BE LIABLE FOR SPECIAL,
# INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THE SOFTWARE
# AND DOCUMENTATION.

CC=gcc -g
CXX=g++ -g
CFLAGS=
LDFLAGS=
YFLAGS=
OBJS= gram.o x_arrayc.o x_arrayp.o code.o iburg.o
GENERICS = x_array.gen hash.gen

.SUFFIXES: .c .impl .h .int

.int.h:
	echo "define(TARGET,interface)" | m4 - $(GENERICS) $*.int >$*.h

.impl.c:
	echo "define(TARGET,implementation)" | m4 - $(GENERICS) $*.impl >$*.c

.c.o:
		$(CC) -c $<

olive:	$(OBJS)
		$(CC) -o olive $(LDFLAGS) $(OBJS)

$(OBJS):	tree.h code.h

test:	olive sample4.brg
		./olive -I sample4.brg; $(CXX) sample4.cpp; ./a.out

clean:
		rm -f a.out *.o tree.h sample*.h *~ core sample*.cpp olive gram.c

clobber:	clean
		rm -f y.tab.c olive
