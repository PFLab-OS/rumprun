ASMS=	arch/aarch64/locore.S

SRCS+=	arch/aarch64/machdep.c
SRCS+=	arch/aarch64/serialcons.c

CPPFLAGS+=	-Iarch/aarch64

.PHONY: archdirs
archdirs:
	mkdir -p ${RROBJ}/platform/arch/aarch64
