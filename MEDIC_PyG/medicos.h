
#ifndef MEDICOS_H
#define MEDICOS_H

#include "usuarios.h"

/* ======================
* Acciones del médico
* ====================== */

/* El médico ve las citas disponibles */
void medico_ver_citas_disponibles(void);

/* El médico escribe una receta para una cita */
void medico_escribir_receta(void);

#endif /* MEDICOS_H */
