.set noreorder # If we're writing assembly, why would we want this?

.section .text.startup

.global _start
_start:
	j JUMP
	nop
