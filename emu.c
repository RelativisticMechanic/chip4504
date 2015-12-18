// CHIP4504 emulator
#define CHIP4504_SEGMENT 0x3000
#define VIDEO_SEGMENT 0xA000

#define PITCH 320 
#define VGA_HEIGHT 200
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 128

#define ACTUAL_W 64
#define ACTUAL_H 32

#define SCHIP_W 128
#define SCHIP_H 64

#define PROG_ORIGIN 0x200
#define MEM_SIZE 0xFFFF+1 // Total addressable memory. 64 kilobytes (0xX << 12) | 0xYYY -> X = SEG (0x0-0xF), YYY = Offset.

#define MAX_NESTING 0xFF+1
#define PORT_MAX 0xFF+1

#define ENCODECSIP(seg, off) (((seg) << 12)+(off))
#define DECODESEG(val) (((val) & 0xF000) >> 12)
#define DECODEIP(val) (((val) & 0xFFF))

#define DEFAULT_FREQ 440
// typedefs
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

unsigned int next = 0x7373;
// structs
typedef struct {
	// General purpose registers
	uint8_t V[0x10];
	// Interrupt addresses.
	uint16_t INT[0x10];
	// Ports.
	uint8_t PORT[PORT_MAX];
	// Index Register
	uint16_t I;
	// Code Index register
	uint16_t CI;
	// Instruction Pointer and Stack Pointer
	uint16_t IP;
	uint16_t SP;
	uint8_t DSP_SEG;
	uint16_t DSP_OFF;
	// Delay timer and Sound Timer
	uint8_t DT;
	uint8_t ST;
	// Code Segment
	uint8_t CS;
	// Data segment
	uint8_t DS;
	// Draw flag.
	uint8_t DF;
	// extended mode flag
	uint8_t EX;
	// Graphics. only [32][64] is used when in normal mode..
	uint8_t GFX[64][128];
	uint8_t KEY[0x10];
	// Stack Memory
	// 16-bit addresses for returning.
	uint16_t STK[MAX_NESTING];
	// HP48 flag system emulation
	uint8_t HP48F[8];
	//  screen divisor..
	uint8_t divisor;
	// screen length..
	uint8_t GFX_w;
	uint8_t GFX_h;
} chip8_cpu;

// some data
chip8_cpu cpu;

unsigned char chip8_font[5*0x10] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
unsigned char superchip8_font[10*0x10] = {
	0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,	// 0
	0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF,	// 1
	0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF,	// 2
	0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,	// 3
	0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, // 4
	0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,	// 5
	0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF,	// 6
	0xFF, 0xFF, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, // 7
	0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF,	// 8
	0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,	// 9
	0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, // A
	0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, // B
	0x3C, 0xFF, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xFF, 0x3C, // C
	0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC, // D
	0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // E
	0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0  // F
};
// Helper functions
uint16_t ror16(uint16_t word, unsigned int shift) {
	return (word >> shift) | (word << (16 - shift));
}
uint16_t rol16(uint16_t word, unsigned int shift){
	return (word << shift) | (word >> (16 - shift));
}
uint8_t peek_byte(uint16_t seg, uint16_t off) {
	asm("push ds\n"
		"xor ax, ax\n"
		"mov ds, [bp + 4]\n"
		"mov bx, [bp + 6]\n"
		"mov al, byte [bx]\n"
		"pop ds\n"
		);
}
uint16_t peek_word(uint16_t seg, uint16_t off) {
	asm("push ds\n"
		"xor ax, ax\n"
		"mov ds, [bp + 4]\n"
		"mov bx, [bp + 6]\n"
		"mov ax, word [bx]\n"
		"pop ds"
		);
}
void poke_byte(uint16_t seg, uint16_t off, uint8_t byte) {
	asm("push ds\n"
		"mov al, byte [bp + 8]\n"
		"mov ds, [bp + 4]\n"
		"mov bx, [bp + 6]\n"
		"mov byte [bx], al\n"
		"pop ds\n"
		);
}
void poke_word(uint16_t seg, uint16_t off, uint16_t word) {
	asm("push ds\n"
		"mov ax, word [bp + 8]\n"
		"mov ds, [bp + 4]\n"
		"mov bx, [bp + 6]\n"
		"mov word [bx], ax\n"
		"pop ds\n"
		);
}
void graphics_init(void) {
	asm("mov ah, 0x00\n"
		"mov al, 0x13\n"
		"int 0x10\n"
		);
}
void graphics_exit(void) {
	asm ("mov ah, 0x00\n"
		"mov al, 0x03\n"
		"int 0x10\n"
		);
}
uint16_t keywait(void) {
	asm("mov ah, 0x10\n"
		"int 0x16\n"
		);
}
uint16_t keycheck(void) {
	asm("mov ah, 0x01\n"
		"int 0x16\n"
		"jz .finish\n"
		"xor ax, ax\n"
		"int 0x16\n"
		".finish:"
		);
}
void emit_beep(unsigned int freq) {
	asm("mov ax, [bp + 4]\n"
		"call os_speaker_tone\n"
		);
} 
int rand(void) {
	unsigned int random_shit = get_ticks();
    next = next * 0xFF3F + 12345 * random_shit;
    return ((next + random_shit)+get_ticks()*2+random_shit/get_ticks());
}
 
void srand( unsigned int seed ) {
    next = seed;
}
void memset(void* s, int c, unsigned int n) {
	char* p = (char*)s;
	while(n--) {
		p[n] = c;
	}
}
void out_byte(uint8_t port, uint8_t byte) {
	asm("mov al, byte [bp + 6]\n"
		"xor dx, dx\n"
		"mov dl, byte [bp + 4]\n"
		"out dx, al\n"
		);
}
uint16_t get_ticks(void) {
	asm("mov ah, 0x00\n"
		"int 0x1A\n"
		"mov ax, dx\n"
		);
}
// sets timer to 60hz
void init_ticks(void) {
	uint16_t freq = 19886; // 1193180 / 60
	out_byte(0x43, 0x36);
	uint8_t l = freq & 0xFF;
	uint8_t h = (freq >> 8) & 0xFF;
	out_byte(0x40, l);
	out_byte(0x40, h);
}
void delay(unsigned int ms) {
	asm("mov ax, [bp + 4]\n"
		"call os_pause\n"
		);
}
void clear_chip8(chip8_cpu* cpu) {
	memset((void*)cpu->GFX, 0, sizeof(cpu->GFX));
	cpu->DF = 1;
}
void timers_chip8(chip8_cpu* cpu) {
	if(cpu->DT > 0) {
		cpu->DT -= 1;
	}
	if(cpu->ST > 0) {
		cpu->ST -= 1;
		emit_beep(DEFAULT_FREQ);
	}
}
void draw_chip8(chip8_cpu* cpu) {
	for(unsigned int i = 0; i < SCREEN_HEIGHT*SCREEN_WIDTH; i++) {
		poke_byte(VIDEO_SEGMENT, i, 0x00);
	}
	for(unsigned int i = 0; i < SCREEN_HEIGHT; i++) {
		for(unsigned int j = 0; j < SCREEN_WIDTH; j++) {
			poke_byte(VIDEO_SEGMENT, ((i+(VGA_HEIGHT-SCREEN_HEIGHT)/2)*PITCH)+(j+(PITCH-SCREEN_HEIGHT)/6), cpu->GFX[i/cpu->divisor][j/cpu->divisor] ? 0x0F : 0x00); 
		}
	}
}
void draw_sprite(chip8_cpu* cpu, int vx, int vy, int n) {
	cpu->V[0x0F] = 0;
	unsigned int pixel = 0;
	for(int y = 0; y < n; y++) {
		pixel = peek_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I + y);
		for(int x = 0; x < 8; x++) {
			if(pixel & (0x80 >> x)) {
				if(cpu->GFX[(y+vy)%ACTUAL_H][(x+vx)%ACTUAL_W]) {
					cpu->V[0x0F] = 1;
				}
				cpu->GFX[(y+vy)%ACTUAL_H][(x+vx)%ACTUAL_W] ^= 1; 
			}
		}
	}
}
void draw_sprite_ext(chip8_cpu* cpu, int vx, int vy, int n) {
	cpu->V[0x0F] = 0;
	if (!n) {
		for (int y = 0; y < 16; y++) {
			unsigned char data = peek_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I + y*2);
			for (int x = 0; x < 8; x++) {
				if((data & (0x80 >> x)) != 0) {
					if (cpu->GFX[(vy + y)%SCHIP_H][(vx + x)%SCHIP_W] == 1) {
						cpu->V[0xF] = 1;
					}
					cpu->GFX[(vy+y)%SCHIP_H][(vx+x)%SCHIP_W] ^= 1;
				}
			}
			data = peek_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I + 1 + y*2);
			for (int x = 0; x < 8; x++) {
				if((data & (0x80 >> x)) != 0) {
					if (cpu->GFX[(vy + y)%SCHIP_H][(vx + x + 8)%SCHIP_W] == 1) { 
						cpu->V[0xF] = 1;
					}
					cpu->GFX[(vy + y)%SCHIP_H][(vx + x + 8)%SCHIP_W] ^= 1;
				}
			}
		}
	} else {
		for (int y = 0; y < n; y++) {
			unsigned char data = peek_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I + y);
			for (int x = 0; x < 8; x++) {
				if((data & (0x80 >> x)) != 0) {
					if (cpu->GFX[(vy + y)%SCHIP_H][(vx + x)%SCHIP_W] == 1) {
						cpu->V[0xF] = 1;
					}
					cpu->GFX[(vy + y)%SCHIP_H][(vx + x)%SCHIP_W] ^= 1;
				}
			}
		}
	}
}
unsigned int get_key_status(void) {
	asm("mov ah, 0x11\n"
		"int 0x16\n"
		"pushf\n"
		"xor ax, ax\n"
		"popf\n"
		"jz .no_key\n"
		"mov ax, 1\n"
		".no_key:\n"
		);
}
void keys_chip8(chip8_cpu* cpu) {
	unsigned int mix = keycheck();
	unsigned char key = mix & 0x00FF;
	if(key == 27) {
		handle_exit(0);
	}
	if(key == 0) {
		return;
	}
	for(int i = 0; i < 0x10; i++) {
		cpu->KEY[i] = 0;
	}
	switch(key) {
		case '1': cpu->KEY[1] = 1; break;
		case '2': cpu->KEY[2] = 1; break;
		case '3': cpu->KEY[3] = 1; break;
		case '4': cpu->KEY[0xC] = 1; break;
		case 'q': cpu->KEY[4] = 1; break;
		case 'w': cpu->KEY[5] = 1; break;
		case 'e': cpu->KEY[6] = 1; break;
		case 'r': cpu->KEY[0xD] = 1; break;
		case 'a': cpu->KEY[7] = 1; break;
		case 's': cpu->KEY[8] = 1; break;
		case 'd': cpu->KEY[9] = 1; break;
		case 'f': cpu->KEY[0xE] = 1; break;
		case 'z': cpu->KEY[0xA] = 1; break;
		case 'x': cpu->KEY[0] = 1; break;
		case 'c': cpu->KEY[0xB] = 1; break;
		case 'v': cpu->KEY[0xF] = 1; break;
		default: break;
	}
}
void init_chip8(chip8_cpu* cpu) {
	//! load font into memory.. from 0 to 80.
	for(int i = 0; i < (5*0x10); i++) {
		poke_byte(CHIP4504_SEGMENT, i, chip8_font[i]); 
	}
	//! load superchip font.
	for(int i = 0; i < (10*0x10); i++) {
		poke_byte(CHIP4504_SEGMENT, i+80, superchip8_font[i]);
	}
	//! clear stack
	memset((void*)cpu->STK, 0, sizeof(cpu->STK));
	memset((void*)cpu->GFX, 0, sizeof(cpu->GFX));
	memset((void*)cpu->V, 0, sizeof(cpu->V));
	cpu->IP = PROG_ORIGIN;
	cpu->SP = 0;
	//! Start in compatibility mode, set CS and DS to 0.
	cpu->CS = 0;
	cpu->DS = 0;
	cpu->GFX_w = ACTUAL_W;
	cpu->GFX_h = ACTUAL_H;
	cpu->divisor = SCREEN_HEIGHT/cpu->GFX_h;
	cpu->EX = 0;
}
// main code
void handle_exit(int code) {
	graphics_exit();
	asm("jmp c4504_ui\n");
}
void execute_chip8(chip8_cpu* cpu) {
	uint16_t opcode = ((peek_byte(CHIP4504_SEGMENT, (cpu->CS << 12) + cpu->IP) << 8) | peek_byte(CHIP4504_SEGMENT, (cpu->CS << 12) + cpu->IP+1));
	cpu->DF = 0;
	// NOP
	if(opcode == 0x0000) cpu->IP += 2;
	// EXTENDED OPCODE: 0xFFFF.
	switch(opcode & 0xF000) {
		case 0x0000:
			if((opcode & 0x00F0) == 0x90) {
				// OUT VX, VY
				// Output VX to port VY.
				// 0x0X9Y
				cpu->PORT[cpu->V[opcode & 0x000F]] = cpu->V[(opcode & 0x00F0) >> 8];
				cpu->IP += 2;
				break;
			} else if((opcode & 0x00F0) == 0xA0) {
				// IN VX, VY
				// Input into VX from Port VY.
				// 0x0XAY
				cpu->V[(opcode & 0x0F00) >> 8] = cpu->PORT[(cpu->V[opcode & 0x000F])];
				cpu->IP += 2;
				break;
			} else if((opcode & 0x00F0) == 0xB0) {
				//! 0x00BN
				//! set interrupt N to DS:CI
				cpu->INT[opcode & 0x000F] = ENCODECSIP(cpu->DS, cpu->CI);
				cpu->IP += 2;
				break; 
			} else if((opcode & 0x00F0) == 0xC0) {
				// SCHIP - Scroll screen down N pixels
				// SCD
				// 0x00CN
				uint8_t n = opcode & 0x000F;
				for(unsigned int y = SCHIP_H - 1; y > n; y--) {
					for(unsigned int x = 0; x < SCHIP_W; x++) {
						cpu->GFX[y][x] = cpu->GFX[y-n][x];
					}
				}
				for(unsigned int y = 0; y < n; y++) {
					for(unsigned int x = 0; x < SCHIP_W; x++) {
						cpu->GFX[y][x] = 0;
					}
				}
				cpu->DF = 1;
				cpu->IP += 2;
				break;
			} else if((opcode & 0x00F0) == 0xD0) {
				//! cause interrupt.
				//! INT X
				// 0x00DX
				cpu->STK[(cpu->SP)&0xFF] = ENCODECSIP(cpu->CS, cpu->IP);
				cpu->SP += 1;
				cpu->IP = DECODEIP(cpu->INT[opcode & 0x000F]);
				cpu->CS = DECODESEG(cpu->INT[opcode & 0x000F]);
				break;
			}
			switch(opcode & 0x00FF) {
				case 0xE0:
					clear_chip8(cpu);
					cpu->IP += 2;
					break;
				/* EXTENDED OPCODES */
				case 0xE1:
					// set data segment to register.
					// MOV DS, VX
					// 0x0XE1
					cpu->DS = cpu->V[(opcode & 0x0F00) >> 8] & 0x0F;
					cpu->IP += 2;
					break;
				case 0xE2:
					// set register to data segment
					// MOV VX, DS
					// 0x0XE2
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->DS & 0x0F;
					cpu->IP += 2;
					break;
				case 0xE3:
					// set register to code segment.
					// MOV VX, CS
					// 0x0XE3
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->CS;
					cpu->IP += 2;
					break;
				case 0xE4:
					// Set data segment to code segment. 
					// MOV DS, CS
					// 0x00E4
					cpu->DS = cpu->CS;
					cpu->IP += 2;
					break;
				case 0xE5:
					// Set data stack segment to register. 
					// MOV SS, VX
					// 0x0XE5
					cpu->DSP_SEG = cpu->V[(opcode & 0x0F00) >> 8];
					cpu->IP += 2;
					break;
				case 0xE6:
					// Set data stack offset to index.
					// MOV SP, I
					// 0x00E6
					cpu->DSP_OFF = (cpu->I & 0x0FFF);
					cpu->IP += 2;
					break;
				case 0xE7:
					// push register to DSP_SEG:DSP_OFF and decrement by 1.
					// PUSH VX
					// 0x0XE7
					cpu->DSP_OFF = (uint16_t)((cpu->DSP_OFF - 1) & 0x0FFF);
					poke_byte(CHIP4504_SEGMENT, (uint16_t)(((cpu->DSP_SEG & 0x0F) >> 12) | (cpu->DSP_OFF & 0x0FFF)), cpu->V[(opcode & 0x0F00) >> 8]);
					cpu->IP += 2;
					break;
				case 0xE8:
					// pop register from DSP_SEG:DSP_OFF and increment by 1.
					// POP VX
					// 0x0XE8
					cpu->V[(opcode & 0x0F00) >> 8] = peek_byte(CHIP4504_SEGMENT, (uint16_t)(((cpu->DSP_SEG & 0x0F) >> 12) | (cpu->DSP_OFF & 0x0FFF)));
					cpu->DSP_OFF = (uint16_t)((cpu->DSP_OFF + 1) & 0x0FFF);
					cpu->IP += 2;
					break;
				case 0xE9:
					// Set CI to I
					// MOV CI, I
					// 0x00E9
					cpu->CI = cpu->I;
					cpu->IP += 2;
					break;
				case 0xEA:
					// Set I to CI
					// MOV I, CI
					// 0x00EA
					cpu->I = cpu->CI;
					cpu->IP += 2;
					break;
				case 0xEB:
					// PUSH I
					cpu->DSP_OFF = (uint16_t)((cpu->DSP_OFF - 2) & 0x0FFF);
					poke_word(CHIP4504_SEGMENT, (uint16_t)(((cpu->DSP_SEG & 0x0F) >> 12) | (cpu->DSP_OFF & 0x0FFF)), cpu->I);
					cpu->IP += 2;
					break;
				case 0xEC:
					// POP I
					cpu->I = peek_word(CHIP4504_SEGMENT, (uint16_t)(((cpu->DSP_SEG & 0x0F) >> 12) | (cpu->DSP_OFF & 0x0FFF)));
					cpu->DSP_OFF = (uint16_t)((cpu->DSP_OFF + 2) & 0x0FFF);
					cpu->IP += 2;
					break;
				case 0xED:
					// MOV I, SP
					cpu->I = cpu->DSP_OFF;
					cpu->IP += 2;
					break;
				case 0xEE:
					if(cpu->SP == 0) {
						// ERR HANDLE
					}
					cpu->SP -= 1;
					cpu->IP = DECODEIP(cpu->STK[(cpu->SP)&0xFF])+2;
					cpu->CS = DECODESEG(cpu->STK[cpu->SP&0xFF]);
					break;
				case 0xFB:
					if(!cpu->EX) {
						// ERR HANDLE
						//fprintf(stderr, "WARNING: scrolling unimplemented for normal mode. enable extended mode in game before using this feature\n");
					}
					// scroll display 4 pixels right
					for(int y = 0; y < SCHIP_H; y++) {
						for(int x = SCHIP_W - 1; x > 3; x--) {
							cpu->GFX[y][x] = cpu->GFX[y][x-4]; 
						}
						cpu->GFX[y][0] = 0;
						cpu->GFX[y][1] = 0;
						cpu->GFX[y][2] = 0;
						cpu->GFX[y][3] = 0;
					}
					cpu->DF = 1;
					cpu->IP += 2;
					break;
				case 0xFC:
					// scroll display 4 pixels left
					// SCD 
					for (int y = 0; y < SCHIP_H; y++) {
						for (int x = 0; x < SCHIP_W - 4; x++) {
							cpu->GFX[y][x] = cpu->GFX[y][x+4];
						}
						cpu->GFX[y][124] = 0;
						cpu->GFX[y][125] = 0;
						cpu->GFX[y][126] = 0;
						cpu->GFX[y][127] = 0;
					}
					cpu->DF = 1;
					cpu->IP += 2;
					break;
				case 0xFD:
					handle_exit(0);
					break;
				case 0xFE:
					cpu->EX = 0;
					cpu->GFX_w = ACTUAL_W;
					cpu->GFX_h = ACTUAL_H;
					cpu->divisor = SCREEN_WIDTH/ACTUAL_W;
					cpu->IP += 2;
				case 0xFF:
					cpu->EX = 1;
					cpu->GFX_w = SCHIP_W;
					cpu->GFX_h = SCHIP_H;
					cpu->divisor = SCREEN_WIDTH/SCHIP_W;
					cpu->IP += 2;
					break;
				default:
					// ERR HANDLE
					//fprintf(stderr, "Error, unknown opcode with nibble : 0x0, %x\n", opcode & 0x00FF);
					handle_exit(-1);
			}
			break;
		case 0x1000:
			// JMP NNN
			// 0x1NNN
			cpu->IP = opcode & 0x0FFF;
			break;
		case 0x2000:
			// CALL NNN
			// 0x2NNN
			if(!(cpu->SP < MAX_NESTING)){ 
				//fprintf(stderr, "Stack overflow. Too many calls\n");
				handle_exit(-1);
			}
			cpu->STK[(cpu->SP)&0xFF] = ENCODECSIP(cpu->CS, cpu->IP);
			cpu->SP += 1;
			cpu->IP = opcode & 0x0FFF;
			break;
		case 0x3000:
			// SKIPE VX, NN
			// 0x3XNN
			if(cpu->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
				cpu->IP += 4;
			} else {
				cpu->IP += 2;
			}
			break;
		case 0x4000:
			// SKIPNE VX, NN
			// 0x4XNN
			if(cpu->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
				cpu->IP += 4;
			} else {
				cpu->IP += 2;
			}
			break;
		case 0x5000:
			// SKIPE VX, VY
			// 0x5XY0
			if(cpu->V[(opcode & 0x0F00) >> 8] == (cpu->V[(opcode & 0x00F0) >> 4])) {
				cpu->IP += 4;
			} else {
				cpu->IP += 2;
			}
			break;
		case 0x6000:
			// MOV VX, NN
			// 0x6XNN
			cpu->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			cpu->IP += 2;
			break;
		case 0x7000:
			// ADD VX, NN
			// 0x7XNN
			cpu->V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			cpu->IP += 2;
			break;
		case 0x8000:
			switch(opcode & 0x000F) {
				// MOV VX, VY
				// 0x8XY0
				case 0x0:
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x00F0) >> 4];
					break;
				// OR VX, VY
				// 0x8XY1
				case 0x01:
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] | cpu->V[(opcode & 0x00F0) >> 4];
					break;
				// AND VX, VY
				// 0x8XY2
				case 0x02:
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] & cpu->V[(opcode & 0x00F0) >> 4];
					break;
				// XOR VX, VY
				// 0x8XY3
				case 0x03:
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] ^ cpu->V[(opcode & 0x00F0) >> 4];
					break;
				// ADD VX, VY (sets carry flag VF)
				// 0x8XY4
				case 0x04:
					if(((int)cpu->V[(opcode & 0x0F00) >> 8] + (int)cpu->V[(opcode & 0x00F0) >> 4]) < 256) {
						cpu->V[0x0F] = 0;
					} else {
						cpu->V[0x0F] = 1;
					}
					cpu->V[(opcode & 0x0F00) >> 8] += cpu->V[(opcode & 0x00F0) >> 4];
					break;
				// SUB VX, VY
				// 0x8XY5
				// sets carry VF.
				case 0x05:
					if(((int)cpu->V[(opcode & 0x0F00) >> 8] - (int)cpu->V[(opcode & 0x00F0) >> 4]) >= 0) {
						cpu->V[0x0F] = 1;
					} else {
						cpu->V[0x0F] &= 0;
					}
					cpu->V[(opcode & 0x0F00) >> 8] -= cpu->V[(opcode & 0x00F0) >> 4];
					break;
				case 0x06:
				// SHR1 VX
				// 0x8X06
				// (sets carry VF)
					cpu->V[0x0F] = cpu->V[(opcode & 0x0F00) >> 8] & 7;
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] >> 1;
					break;
				// ...
				case 0x07:
					if(((int)cpu->V[(opcode & 0x00F0) >> 4] - (int)cpu->V[(opcode & 0x0F00) >> 8]) > 0) {
						cpu->V[0x0F] = 1;
					} else {
						cpu->V[0x0F] = 0;
					}
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x00F0) >> 4] - cpu->V[(opcode & 0x0F00) >> 8];
					break;
				case 0x08:
					// SHL VX, VY
					// 0x8XY8
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] << cpu->V[(opcode & 0x00F0) >> 4];
					break;
				case 0x09:
					// 0x8XY9
					// SHR VX, VY
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] >> cpu->V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0A:
					// 0x8XYA
					// ROL VX, VY
					cpu->V[(opcode & 0x0F00) >> 8] = rol16(cpu->V[(opcode & 0x0F00) >> 8], cpu->V[(opcode & 0x00F0) >> 4]);
					break;
				case 0x0B:
					// 0x8XYB
					// ROR VX, VY
					cpu->V[(opcode & 0x0F00) >> 8] = ror16(cpu->V[(opcode & 0x0F00) >> 8], cpu->V[(opcode & 0x00F0) >> 4]);
					break;
				case 0x0C:
					// 0x8XYC
					// XCHG VX, VY
					// Exchange VX with VY.
					{
						uint16_t VX = cpu->V[(opcode & 0x0F00) >> 8];
						uint16_t VY = cpu->V[(opcode & 0x00F0) >> 4];
						cpu->V[(opcode & 0x0F00) >> 8] = VY;
						cpu->V[(opcode & 0x00F0) >> 4] = VX;
					}
					break;
				case 0x0D:
					switch((opcode & 0x00F0) >> 4) {
						case 0x00:
							// 0x8X0D
							// NOT VX
							cpu->V[(opcode & 0x0F00) >> 8] = !(cpu->V[(opcode & 0x0F00) >> 8]);
							break;
						case 0x01:
							// 0x8X1D
							// NEG VX
							cpu->V[(opcode & 0x0F00) >> 8] = ~(cpu->V[(opcode & 0x0F00) >> 8]);
							break;
					}
					break;
				// SHL1 VX
				// 0x8X0E
				case 0x0E:
					cpu->V[0x0F] = cpu->V[(opcode & 0x0F00) >> 8] >> 7;
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->V[(opcode & 0x0F00) >> 8] << 1;
					break;
				default:
					// ERR HANDLE
					//fprintf(stderr, "Invalid fourth nibble for opcode 0x8000 %x\n", opcode);
					handle_exit(-1);
					break;
			}
			cpu->IP += 2;
			break;
		case 0x9000:
			// SKIPNE VX, VY
			// 0x9XY0
			if(cpu->V[(opcode & 0x0F00) >> 8] != cpu->V[(opcode & 0x00F0) >> 4]) {
				cpu->IP += 4;
			} else {
				cpu->IP += 2;
			}
			break;
		case 0xA000:
			// MOV I, NNN
			// 0xANNN
			cpu->I = opcode & 0x0FFF;
			cpu->IP += 2;
			break;
		case 0xB000:
			// JMP0 NNN
			// 0xBNNN
			cpu->IP = (uint16_t)((opcode & 0x0FFF) + cpu->V[0]) & 0x0FFF;
			break;
		case 0xC000:
			// RAND VX, NN
			// 0xCXNN
			cpu->V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
			cpu->IP += 2;
			break;
		case 0xD000:
			// DRW VX, VY, N
			// 0xDXYN
			if(cpu->EX) {
				draw_sprite_ext(cpu, cpu->V[(opcode & 0x0F00) >> 8], cpu->V[(opcode & 0x00F0) >> 4], opcode & 0x000F);
			} else {
				draw_sprite(cpu, cpu->V[(opcode & 0x0F00) >> 8], cpu->V[(opcode & 0x00F0) >> 4], opcode & 0x000F);
			}
			cpu->DF = 1;
			cpu->IP += 2;
			break;
		case 0xE000:
			switch(opcode & 0x00FF) {
				// SKIPDN VX
				// 0xEX9E 
				case 0x9E:
					if(cpu->KEY[cpu->V[(opcode & 0x0F00) >> 8]] == 1) {
						cpu->IP += 4;
					} else {
						cpu->IP += 2;
					}
					break;
				// SKIPUP VX
				// 0xEXA1
				case 0xA1:
					if((cpu->KEY[cpu->V[(opcode & 0x0F00) >> 8]]) == 1) {
						cpu->IP += 2;
					} else {
						cpu->IP += 4;
					}
					break;
				default:
					//fprintf(stderr, "Error, invalid opcode for 0xE000\n");
					handle_exit(-1);
			}
			break;
		case 0xF000:
			if ((opcode & 0x00F0) == 0xA0) {
				//! 0xFXAY Set CI (Code Index) register: to (VX << 8) | VY (MOV CI, VX, VY) 
				cpu->CI = ((cpu->V[(opcode & 0x0F00) >> 8] << 8) | cpu->V[opcode & 0x000F]) & 0x0FFF;
				cpu->IP += 2;
				break;
			} else if((opcode & 0x00F0) == 0xB0) {
				//! 0xFXBY, Set CS:IP to: VX:CI+VY (JMPF VX, VY)
				cpu->CS = cpu->V[(opcode & 0x0F00) >> 8];
				cpu->IP = (cpu->CI + cpu->V[opcode & 0x000F]) & 0x0FFF; 
				break;
			} else if((opcode & 0x00F0) == 0xC0) {
				//! inter segmental call.
				//! 0xFXCY, Push CS:IP, and call VX:CI+VY (CALLF VX, VY)
				cpu->STK[(cpu->SP & 0xFF)] = ENCODECSIP(cpu->CS, cpu->IP);
				cpu->CS = cpu->V[(opcode & 0x0F00) >> 8];
				cpu->IP = (cpu->CI + cpu->V[opcode & 0x000F]) & 0x0FFF;
				break;
			} else if((opcode & 0x00F0) == 0xD0) {
				// MOV CI, NNN
				// 0xF0D0 0x0NNN
				// Sets code index to immediate.
				cpu->IP += 2;
				cpu->CI = (peek_byte(CHIP4504_SEGMENT, (cpu->CS << 12) + cpu->IP << 8) | peek_byte(CHIP4504_SEGMENT, (cpu->CS << 12) + cpu->IP+1)) & 0x0FFF;
				cpu-> IP += 2;
				break;
			}
			switch(opcode & 0x00FF) {
				// MOV VX, DT
				// 0xFX07
				case 0x07:
					cpu->V[(opcode & 0x0F00) >> 8] = cpu->DT;
					cpu->IP += 2;
					break;
				// WAITKEY VX
				// 0xFX0A
				case 0x0A:
					for(int i = 0; i < 0x10; i++) {
						if(cpu->KEY[i]) {
							cpu->V[(opcode & 0x0F00) >> 8] = i;
							cpu->IP += 2;
						}
					}
					break;
				// MOV DT, VX
				// 0xFX15
				case 0x15:
					cpu->DT = cpu->V[(opcode & 0x0F00) >> 8];
					cpu->IP += 2;
					break;
				// MOV ST, VX
				// 0xFX18
				case 0x18:
					cpu->ST = cpu->V[(opcode & 0x0F00) >> 8];
					cpu->IP += 2;
					break;
				case 0x1E:
				// ADD I, VX
				// 0xFX1E
					cpu->I += cpu->V[(opcode & 0x0F00) >> 8];
					cpu->IP += 2;
					break;
				case 0x1F:
				// 0xFX1F
				// ADD CI, VX
					cpu->CI += cpu->V[(opcode & 0x0F00) >> 8];
					cpu->IP += 2;
					break;
				// GETSPR VX
				// 0xFX29
				case 0x29:
					cpu->I = cpu->V[(opcode & 0x0F00) >> 8] * 5;
					cpu->IP += 2;
					break;
				// GETSPREX VX
				// 0xFX30
				case 0x30:
					cpu->I = cpu->V[(opcode & 0x0F00) >> 8] * 10 + 80;
					cpu->IP += 2;
					break;
				// STOREBCD VX
				// 0xFX33
				case 0x33:
					poke_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I, cpu->V[(opcode & 0x0F00) >> 8] / 100);
					poke_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I+1, (cpu->V[(opcode & 0x0F00) >> 8] / 10) % 10);
					poke_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I+2, cpu->V[(opcode & 0x0F00) >> 8] % 10);
					cpu->IP += 2;
					break;
				// STORE X
				// Store from V0 - VX to I.
				// 0xFX55
				case 0x55:
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
						poke_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I+i, cpu->V[i]);
					}
					cpu->IP += 2;
					break;
				// LOAD VX
				// 0xFX65
				case 0x65:
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
						 cpu->V[i] = peek_byte(CHIP4504_SEGMENT, (cpu->DS >> 12) + cpu->I+i);
					}
					cpu->IP += 2;
					break;
				// HP48L VX
				// 0xFX85
				case 0x85:
					if(((opcode & 0x0F00) >> 8) > 7) {
						// ERR HANDLE
						//fprintf(stderr, "Error, cannot store more than 8 registers in HP48 flags\n");
						handle_exit(-1);
					}
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
						cpu->V[i] = cpu->HP48F[i];
					}
					cpu->IP += 2;
					break;
				// HP48S VX
				// 0xFX75
				case 0x75:
					if(((opcode & 0x0F00) >> 8) > 7) {
						// ERR HANDLE
						//fprintf(stderr, "Error, cannot store more than 8 registers in HP48 flags\n");
						handle_exit(-1);
					}
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
						cpu->HP48F[i] = cpu->V[i];
					}
					cpu->IP += 2;
					break;
				default:
					// ERR HANDLE
					//fprintf(stderr, "Error, invalid opcode for 0xF000\n");
					handle_exit(-1);
			}
			break;
		default:
			// ERR HANDLE
			//fprintf(stderr, "Error invalid opcode!\n");
			handle_exit(-1);
	}
}
void chip4504_main(void) {
	graphics_init();
	init_ticks();
	init_chip8(&cpu);
	uint16_t last_tick = get_ticks();
	for(;;) {
		execute_chip8(&cpu);
		if(get_ticks() - last_tick >= 1) {
			asm("call os_speaker_off");
			last_tick = get_ticks();
			timers_chip8(&cpu);
		}
		if(cpu.DF == 1) {
			draw_chip8(&cpu);
			keys_chip8(&cpu);
			delay(1);
		}
		keys_chip8(&cpu);
	}
}