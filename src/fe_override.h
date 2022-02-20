#pragma once

void fe_override_load_overrides(const char *path);

char * fe_override_get_override(unsigned int hash);

bool fe_override_safe_to_reload(unsigned int hash);

void fe_override_unload_all();
void fe_override_reload_all();