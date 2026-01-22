
#ifndef CITAS_H
#define CITAS_H

/* ======================
* Rutas de persistencia
* ====================== */
#define RUTA_CITAS_DISP "citas_disponibles.txt"
#define RUTA_CITAS_CONREC "citas_con_receta.txt"
#define RUTA_RECETAS "recetas.txt"

/* Formato (1 línea por registro):
# Medic_L&G Citas v1
# ID|PACIENTE|MEDICO|ESPECIALIDAD|FECHA|HORA
001|Juan Perez|Dra. Gomez|Cardiologia|20/02/2026|09:30
*/

/* ======================
* Validaciones
* ====================== */
/* Retorna 0 si es válida, <0 si hay error */
int citas_validar_fecha(const char *fecha); /* DD/MM/AAAA */
int citas_validar_hora(const char *hora);   /* HH:MM */

/* ======================
* Consultas
* ====================== */
/* Devuelve 1 si hay conflicto, 0 si está libre */
int citas_hay_conflicto(const char *medico,
						const char *fecha,
						const char *hora);

/* ======================
* Operaciones de Citas
* ====================== */
int citas_agendar(const char *paciente,
				  const char *medico,
				  const char *especialidad,
				  const char *fecha,
				  const char *hora);
int citas_reagendar(const char *id_cita,
					const char *nueva_fecha,
					const char *nueva_hora);
int citas_borrar_por_id(const char *id_cita);
int citas_borrar_por_medico(const char *medico);
int citas_borrar_por_paciente(const char *paciente);

/* ======================
* Listados (sin formato)
* ====================== */
/* Estas funciones SOLO imprimen datos crudos (sin UI bonita) */
void citas_listar_disponibles(void);
void citas_listar_con_receta(void);

/* ======================
* Recetas
* ====================== */
int  citas_crear_receta(const char *id_cita,
						const char *texto_receta);
void recetas_listar_por_paciente(const char *paciente);

#endif /* CITAS_H */
