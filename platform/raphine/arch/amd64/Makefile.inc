ASMS=	arch/amd64/locore.S

SRCS+=	arch/amd64/machdep.c
SRCS+=	arch/x86/clock.c
SRCS+=  arch/x86/x86_subr.c

CFLAGS+=	-mno-sse -mno-mmx

# squash all interrupts to a single level to
# avoid having to figure out routing
CFLAGS+=	-DBMK_SCREW_INTERRUPT_ROUTING

.PHONY: archdirs
archdirs:
	mkdir -p ${RROBJ}/platform/arch/amd64

