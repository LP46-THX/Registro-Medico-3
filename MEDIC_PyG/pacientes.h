
#ifndef PACIENTES_H
#define PACIENTES_H

#include "usuarios.h"

/* ======================
* Acciones del paciente
* ====================== */

/* Agenda una cita para el paciente activo */
void paciente_agendar_cita(const Usuarios *usuarios,
						   const Usuario *paciente);

/* Reagenda una cita del paciente */
void paciente_reagendar_cita(void);

/* Muestra las citas disponibles */
void paciente_ver_citas_disponibles(void);

/* Muestra las citas con receta del paciente */
void paciente_ver_mis_recetas(const Usuario *paciente);

#endif /* PACIENTES_H */
