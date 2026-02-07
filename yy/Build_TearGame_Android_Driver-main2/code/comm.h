#ifndef COMM_H
#define COMM_H

typedef struct {
	pid_t a;
	uintptr_t b;
	void *c;
	size_t d;
} X1, *PX1;

typedef struct {
	pid_t a;
	char *b;
	uintptr_t c;
} X2, *PX2;

typedef enum {
	X3_A = 0,
	X3_B = 1,
	X3_C = 2
} X3;

typedef enum {
	X4_A = 0,
	X4_B = 1
} X4;

typedef struct {
	pid_t a;
	uintptr_t b;
	X3 c;
	unsigned int d;
	X4 e;
} X5, *PX5;

typedef struct {
	uint64_t x[31];
	uint64_t sp;
	uint64_t pc;
	uint64_t pstate;
} X6;

typedef struct {
	pid_t a;
	uint64_t b;
	uintptr_t c;
	struct X6 d;
} X7, *PX7;

typedef struct {
	size_t a;
	PX7 b;
} X8, *PX8;

typedef struct {
	pid_t a;
	uintptr_t b;
	int c;
	uint32_t d;
} X9, *PX9;

typedef struct {
	pid_t a;
	uintptr_t b;
	int c;
	uint64_t d;
} X10, *PX10;

typedef struct {
	pid_t a;
	uintptr_t b;
	int c;
	uint32_t d;
} X11, *PX11;

typedef struct {
	pid_t a;
	uintptr_t b;
	int c;
	uint64_t d;
} X12, *PX12;

enum {
	OP_A = 0xA5B6,
	OP_B = 0xC7D8,
	OP_C = 0xE9FA,
	OP_D = 0x1234,
	OP_E = 0x5678,
	OP_F = 0x9ABC,
	OP_G = 0xDEF0,
	OP_H = 0x2468,
	OP_I = 0x1357,
	OP_J = 0x9753
};

#endif /* COMM_H */