#pragma once

// Safe and recommended way to abort in case of failure states
// Attempts to create a recovery file for unsaved work and flush logs
// before shutting down.

// If this process fails, or the panic function is called within itself, it will hard-abort
void panic();