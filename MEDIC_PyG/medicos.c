
/*
* Modulo: medicos.c
* Rol: Acciones del MEDICO
* - Ver citas disponibles
* - Escribir recetas
*
* Formato de citas:
* ID|PACIENTE|MEDICO|ESPECIALIDAD|FECHA|HORA
*
* Formato de recetas:
* ID|PACIENTE|MEDICO|ESPECIALIDAD|FECHA|HORA|RECETA
*/

#include "medicos.h"
#include "citas.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* =========================
* Colores ANSI
* ========================= */
#define CLR_RESET "\x1b[0m"
#define CLR_BLUE  "\x1b[34m"
#define CLR_RED   "\x1b[31m"
#define CLR_GREEN "\x1b[32m"

/* =========================
* Utilidades de mensajes
* ========================= */
static void info(const char *msg){
	printf("%s[INFO]%s %s\n", CLR_BLUE, CLR_RESET, msg);
}
	static void ok(const char *msg){
		printf("%s[OK]%s %s\n", CLR_GREEN, CLR_RESET, msg);
	}
		static void err(const char *msg){
			fprintf(stderr, "%s[ERROR]%s %s\n", CLR_RED, CLR_RESET, msg);
		}
			
			/* =========================
			* Lecturas seguras
			* ========================= */
			static void consumir_linea(void){
				int ch;
				while ((ch = getchar()) != '\n' && ch != EOF) { /* descartar */ }
			}
				
				static void quitar_crlf(char *s){
					if (!s) return;
					s[strcspn(s, "\r\n")] = '\0';
				}
					
					static int leer_linea(const char *prompt, char *out, size_t n){
						if (!out || n == 0) return 1;
						if (prompt){
							printf("%s", prompt);
							fflush(stdout);
						}
						if (!fgets(out, (int)n, stdin)){
							err("Lectura fallida.");
							return 1;
						}
						if (out[strlen(out) - 1] != '\n')
							consumir_linea();
						quitar_crlf(out);
						return 0;
					}
						
						static int es_regresar(const char *s){
							return s && strcmp(s, "0") == 0;
						}
							
							/* =========================
							* Estructura de cita (1 linea)
							* ========================= */
							typedef struct {
								char id[4];
								char paciente[64];
								char medico[64];
								char especialidad[32];
								char fecha[11];
								char hora[6];
							} CitaTxt;
							
							/* =========================
							* Utilidades de tabla
							* ========================= */
							static void linea(char c, int n){
								for (int i = 0; i < n; ++i) putchar(c);
								putchar('\n');
							}
								
								static void print_cell(const char *s, int w){
									int len = s ? (int)strlen(s) : 0;
									if (len > w){
										for (int i = 0; i < w - 1; ++i) putchar(s[i]);
										putchar('…');
									} else {
										printf("%s", s ? s : "");
										for (int i = len; i < w; ++i) putchar(' ');
									}
								}
									
									static void tabla_header(void){
										linea('=', 86);
										printf("|"); print_cell("ID", 4);
										printf("|"); print_cell("Paciente", 18);
										printf("|"); print_cell("Medico", 18);
										printf("|"); print_cell("Especialidad", 16);
										printf("|"); print_cell("Fecha", 12);
										printf("|"); print_cell("Hora", 6);
										printf("|\n");
										linea('-', 86);
									}
										
										static void tabla_row(const CitaTxt *c){
											printf("|"); print_cell(c->id, 4);
											printf("|"); print_cell(c->paciente, 18);
											printf("|"); print_cell(c->medico, 18);
											printf("|"); print_cell(c->especialidad, 16);
											printf("|"); print_cell(c->fecha, 12);
											printf("|"); print_cell(c->hora, 6);
											printf("|\n");
										}
											
											/* =========================
											* Parseo de cita (1 linea)
											* ========================= */
											static int parse_cita_linea(const char *line, CitaTxt *c){
												if (!line || !c) return -1;
												
												CitaTxt tmp;
												memset(&tmp, 0, sizeof(tmp));
												
												int n = sscanf(line,
															   "%3[^|]|%63[^|]|%63[^|]|%31[^|]|%10[^|]|%5[^\n]",
															   tmp.id, tmp.paciente, tmp.medico,
															   tmp.especialidad, tmp.fecha, tmp.hora);
												
												if (n != 6) return -2;
												
												*c = tmp;
												return 0;
											}
												
												/* =========================
												* API – Medico
												* ========================= */
												void medico_ver_citas_disponibles(void){
													FILE *f = fopen(RUTA_CITAS_DISP, "r");
													if (!f){
														info("No hay archivo de citas disponibles.");
														return;
													}
													
													char line[512];
													
													/* Saltar cabeceras */
													while (fgets(line, sizeof(line), f)){
														if (line[0] != '#') break;
													}
													
													if (feof(f)){
														fclose(f);
														info("No hay citas registradas.");
														return;
													}
													
													tabla_header();
													
													do {
														quitar_crlf(line);
														if (line[0] == '\0') continue;
														
														CitaTxt c;
														if (parse_cita_linea(line, &c) == 0){
															tabla_row(&c);
														}
													} while (fgets(line, sizeof(line), f));
													
													linea('=', 86);
													fclose(f);
												}
													
													void medico_escribir_receta(void){
														char id[8];
														char texto[256];
														
														printf("=== Escribir receta ===\n");
														printf("%s(Escriba 0 para REGRESAR)%s\n", CLR_BLUE, CLR_RESET);
														
														if (leer_linea("ID de la cita (ej: 001): ", id, sizeof(id)) != 0)
															return;
														if (es_regresar(id)){
															info("Regresando...");
															return;
														}
														
														if (leer_linea("Texto de la receta: ", texto, sizeof(texto)) != 0)
															return;
														if (es_regresar(texto)){
															info("Regresando...");
															return;
														}
														
														if (texto[0] == '\0'){
															err("La receta no puede estar vacia.");
															return;
														}
														
														int rc = citas_crear_receta(id, texto);
														if (rc == 0){
															ok("Receta creada y cita movida a 'con receta'.");
														} else if (rc == -3){
															err("No se encontro la cita indicada.");
														} else {
															err("No se pudo crear la receta.");
														}
													}
														
