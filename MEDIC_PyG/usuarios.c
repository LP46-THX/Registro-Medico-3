#include "usuarios.h"
#include <stdio.h>
#include <string.h>

/* =========================
* Inicialización
* ========================= */
void usuarios_init(Usuarios *u){
	if (!u) return;
	u->count = 0;
}
	
	/* =========================
	* Utilidad interna
	* ========================= */
	static const char* rol_to_str(RolUsuario r){
		switch (r){
		case ROL_ADMIN:  return "ADMIN";
		case ROL_MEDICO: return "MEDICO";
		default:         return "PACIENTE";
		}
	}
		
		/* =========================
		* CRUD
		* ========================= */
		bool usuarios_agregar(Usuarios *u, Usuario usr){
			if (!u || u->count >= MAX_USUARIOS) return false;
			
			for (int i = 0; i < u->count; i++){
				if (strcmp(u->arr[i].usuario, usr.usuario) == 0)
					return false;
			}
			u->arr[u->count++] = usr;
			return true;
		}
			
			Usuario *usuarios_login(Usuarios *u, const char *usuario, const char *clave){
				if (!u || !usuario || !clave) return NULL;
				
				for (int i = 0; i < u->count; i++){
					if (strcmp(u->arr[i].usuario, usuario) == 0 &&
						strcmp(u->arr[i].clave, clave) == 0){
						return &u->arr[i];
					}
				}
				return NULL;
			}
				
				/* =========================
				* Listado
				* ========================= */
				void usuarios_listar_tabla(const Usuarios *u){
					if (!u || u->count == 0){
						printf("[INFO] No hay usuarios registrados.\n");
						return;
					}
					
					int total = 4 + 10 + 20 + 15 + 6 + 18 + 7;
					for (int i = 0; i < total; i++) putchar('=');
					printf("\n");
					
					printf("%-4s%-10s%-20s%-15s%-6s%-18s\n",
						   "ID","Tipo","Nombre","Usuario","Edad","Especialidad");
					
					for (int i = 0; i < total; i++) putchar('-');
					printf("\n");
					
					for (int i = 0; i < u->count; i++){
						const Usuario *x = &u->arr[i];
						printf("%-4d%-10s%-20s%-15s%-6d%-18s\n",
							   x->id,
							   rol_to_str(x->rol),
							   x->nombre,
							   x->usuario,
							   x->edad,
							   (x->rol == ROL_MEDICO ? x->especialidad : "N/A"));
					}
					
					for (int i = 0; i < total; i++) putchar('=');
					printf("\n");
				}
					
					/* =========================
					* Persistencia
					* ========================= */
					int usuarios_guardar_txt(const char *ruta, const Usuarios *U){
						if (!ruta || !U) return -1;
						FILE *f = fopen(ruta, "w");
						if (!f) return -2;
						
						fprintf(f, "# Medic_L&G Usuarios\n");
						for (int i = 0; i < U->count; i++){
							const Usuario *x = &U->arr[i];
							fprintf(f, "%d\n%s\n%s\n%s\n%d\n%s\n",
									x->id,
									rol_to_str(x->rol),
									x->nombre,
									x->usuario,
									x->edad,
									(x->rol == ROL_MEDICO ? x->especialidad : "N/A"));
						}
						fclose(f);
						return 0;
					}
						
						int usuarios_cargar_txt(const char *ruta, Usuarios *U){
							if (!ruta || !U) return -1;
							
							FILE *f = fopen(ruta, "r");
							if (!f) return -2;          /* ? NO borrar memoria si no existe el archivo */
							
							usuarios_init(U);           /* ? limpiar SOLO si el archivo existe */
							
							char line[256];
							fgets(line, sizeof(line), f); /* cabecera */
							
							while (fgets(line, sizeof(line), f)){
								Usuario u;
								char rol[16];
								
								sscanf(line, "%d\n%15[^\n]\n%63[^\n]\n%31[^\n]\n%d\n%31[^\n]",
									   &u.id, rol, u.nombre, u.usuario, &u.edad, u.especialidad);
								
								if (strcmp(rol, "ADMIN") == 0) u.rol = ROL_ADMIN;
								else if (strcmp(rol, "MEDICO") == 0) u.rol = ROL_MEDICO;
								else u.rol = ROL_PACIENTE;
								
								usuarios_agregar(U, u);
							}
							fclose(f);
							return 0;
						}
							
							void usuarios_asegurar_admin(Usuarios *U){
								for (int i = 0; i < U->count; i++){
									if (U->arr[i].rol == ROL_ADMIN) return;
								}
								Usuario admin = {1, "ADMIN", "admin007", "0007", 0, ROL_ADMIN, ""};
								usuarios_agregar(U, admin);
							}
								
								/* =========================
								* Validación de médico + especialidad
								* ========================= */
								int usuarios_medico_existe(const Usuarios *u,
														   const char *nombre,
														   const char *especialidad){
									if (!u || !nombre || !especialidad) return 0;
									
									for (int i = 0; i < u->count; i++){
										const Usuario *x = &u->arr[i];
										if (x->rol == ROL_MEDICO &&
											strcmp(x->nombre, nombre) == 0 &&
											strcmp(x->especialidad, especialidad) == 0){
											return 1;
										}
									}
									return 0;
								}
									
