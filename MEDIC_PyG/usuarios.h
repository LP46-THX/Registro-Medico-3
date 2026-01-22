
/*
Proyecto: Medic_P&G
Autores: Luis Posso y Gabriel Quinde
Descripcion: Modulo de usuarios con roles, credenciales y compatibilidad.
Nota: Persistencia unificada: "usuarios.txt" (lista de usuarios).
*/
#ifndef USUARIOS_H
#define USUARIOS_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_USUARIOS 256

/* =========================
* Roles del sistema
* ========================= */
typedef enum {
	ROL_ADMIN = 0,
		ROL_MEDICO = 1,
		ROL_PACIENTE = 2
} RolUsuario;

/* Alias de compatibilidad */
#ifndef ROL_RECEPCION
#define ROL_RECEPCION ROL_PACIENTE
#endif

#ifndef ROL_INVITADO
#define ROL_INVITADO ROL_PACIENTE
#endif

/* =========================
* Usuario
* ========================= */
typedef struct {
	int id;
	char nombre[64];
	char usuario[32];
	char clave[32];
	int edad;
	RolUsuario rol;
	char especialidad[32]; /* solo MEDICO */
} Usuario;

/* =========================
* Contenedor
* ========================= */
typedef struct {
	Usuario arr[MAX_USUARIOS];
	int count;
} Usuarios;

/* =========================
* API existente (SIN CAMBIOS)
* ========================= */
void usuarios_init(Usuarios *u);
bool usuarios_agregar(Usuarios *u, Usuario usr);
Usuario *usuarios_buscar_por_id(Usuarios *u, int id);
Usuario *usuarios_buscar_por_usuario(Usuarios *u, const char *usuario);
bool usuarios_eliminar(Usuarios *u, int id);
void usuarios_listar_tabla(const Usuarios *u);

Usuario *usuarios_login(Usuarios *u, const char *usuario, const char *clave);

int usuarios_guardar_txt(const char *ruta, const Usuarios *U);
int usuarios_cargar_txt(const char *ruta, Usuarios *U);
void usuarios_asegurar_admin(Usuarios *U);

/* =========================
* ? NUEVA VALIDACION (AGREGADA)
* ========================= */
/* Retorna 1 si existe un MEDICO con ese nombre y especialidad */
int usuarios_medico_existe(const Usuarios *u,
						   const char *nombre,
						   const char *especialidad);

#endif /* USUARIOS_H */
