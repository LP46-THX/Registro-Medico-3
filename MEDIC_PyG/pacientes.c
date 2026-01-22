/*
* Modulo: pacientes.c
* Rol: Flujo de acciones del PACIENTE
*/

#include "pacientes.h"
#include "usuarios.h"
#include "citas.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* =========================
* Colores ANSI (coherentes con tu main)
* ========================= */
#define CLR_RESET "\x1b[0m"
#define CLR_BLUE  "\x1b[34m"
#define CLR_RED   "\x1b[31m"
#define CLR_GREEN "\x1b[32m"
static const char* maybe(const char* code){ return code; }

/* Mensajes estandar */
static void info(const char* msg){ printf("%s[INFO]%s %s\n",  maybe(CLR_BLUE), maybe(CLR_RESET), msg); }
static void ok  (const char* msg){ printf("%s[OK]%s %s\n",    maybe(CLR_GREEN),maybe(CLR_RESET), msg); }
static void err (const char* msg){ fprintf(stderr, "%s[ERROR]%s %s\n", maybe(CLR_RED), maybe(CLR_RESET), msg); }

/* =========================
* Lecturas seguras
* ========================= */
static void consumir_linea(void){
	int ch; while ((ch = getchar()) != '\n' && ch != EOF) { /* discard */ }
}
	static void quitar_crlf(char* s){
		size_t n = strcspn(s, "\r\n");
		s[n] = '\0';
	}
		static int leer_linea(const char* prompt, char* out, size_t n){
			if (!out || n==0) return 1;
			if (prompt){ printf("%s", prompt); fflush(stdout); }
			if (!fgets(out, (int)n, stdin)){ err("Lectura fallida."); return 1; }
			if (out[n-1] != '\0' && out[strlen(out)-1] != '\n') consumir_linea();
			quitar_crlf(out);
			return 0;
		}
			static int es_regresar(const char* s){ return (s && strcmp(s, "0")==0); }
			
			/* =========================
			* API del paciente
			* ========================= */
			void paciente_agendar_cita(const Usuarios *usuarios, const Usuario *paciente){
				if (!paciente || !usuarios){
					err("Sesion no valida.");
					return;
				}
				
				char esp[32], doctor[64], fecha[16], hora[16];
				
				printf("=== Agendar cita ===\n");
				printf("%s(Escriba 0 para REGRESAR en cualquier paso)%s\n", maybe(CLR_BLUE), maybe(CLR_RESET));
				
				/* 1) Especialidad */
				if (leer_linea("Especialidad: ", esp, sizeof(esp))!=0) return;
				if (es_regresar(esp)) return;
				
				/* 2) Médico */
				if (leer_linea("Nombre del medico: ", doctor, sizeof(doctor))!=0) return;
				if (es_regresar(doctor)) return;
				
				/* ?? VALIDACIÓN BLOQUEANTE: existe médico y especialidad coincidente */
				if (!usuarios_medico_existe(usuarios, doctor, esp)){
					err("El medico no existe o no pertenece a esa especialidad.");
					return; /* ? NO pedir fecha/hora, NO agendar */
				}
				
				/* 3) Fecha */
				for(;;){
					if (leer_linea("Fecha (DD/MM/AAAA): ", fecha, sizeof(fecha))!=0) return;
					if (es_regresar(fecha)) return;
					if (citas_validar_fecha(fecha)==0) break;
					err("Fecha invalida.");
				}
				
				/* 4) Hora */
				for(;;){
					if (leer_linea("Hora (HH:MM): ", hora, sizeof(hora))!=0) return;
					if (es_regresar(hora)) return;
					if (citas_validar_hora(hora)==0) break;
					err("Hora invalida.");
				}
				
				/* 5) Agendar */
				int rc = citas_agendar(paciente->nombre, doctor, esp, fecha, hora);
				if (rc==0) { ok("Cita agendada."); }
				else if (rc==-3) { err("Conflicto: el medico ya tiene una cita en ese horario."); }
				else { err("No se pudo agendar (error interno)."); }
			}
				
				void paciente_reagendar_cita(void){
					char id[8], fecha[16], hora[16];
					
					printf("=== Reagendar cita ===\n");
					printf("%s(Escriba 0 para REGRESAR en cualquier paso)%s\n", maybe(CLR_BLUE), maybe(CLR_RESET));
					
					if (leer_linea("ID de la cita (ej: 001): ", id, sizeof(id))!=0) return;
					if (es_regresar(id)) { info("Regresando..."); return; }
					
					for(;;){
						if (leer_linea("Nueva fecha (DD/MM/AAAA): ", fecha, sizeof(fecha))!=0) return;
						if (es_regresar(fecha)) { info("Regresando..."); return; }
						if (citas_validar_fecha(fecha)==0) break;
						err("Fecha invalida.");
					}
					for(;;){
						if (leer_linea("Nueva hora (HH:MM): ", hora, sizeof(hora))!=0) return;
						if (es_regresar(hora)) { info("Regresando..."); return; }
						if (citas_validar_hora(hora)==0) break;
						err("Hora invalida.");
					}
					
					int rc = citas_reagendar(id, fecha, hora);
					if (rc==0) ok("Cita reagendada.");
					else if (rc==-5) err("Cita no encontrada.");
					else if (rc==-6) err("Conflicto: el medico ya tiene una cita en ese horario.");
					else err("No se pudo reagendar (error interno).");
				}
					
					void paciente_ver_citas_disponibles(void){
						printf("\n=== Citas disponibles ===\n");
						citas_listar_disponibles();
					}
						
						void paciente_ver_mis_recetas(const Usuario *paciente){
							if (!paciente){ err("Sesion no valida."); return; }
							printf("\n=== Mis recetas ===\n");
							recetas_listar_por_paciente(paciente->nombre);
						}
							
