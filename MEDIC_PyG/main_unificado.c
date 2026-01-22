
/*
* Proyecto: Medic_L&G
* Archivo: main_unificado.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "usuarios.h"
#include "pacientes.h"
#include "medicos.h"
#include "citas.h"

#define RUTA_USUARIOS "usuarios.txt"

/* =========================
* Colores ANSI
* ========================= */
#define CLR_RESET  "\x1b[0m"
#define CLR_BLUE   "\x1b[34m"
#define CLR_RED    "\x1b[31m"
#define CLR_GREEN  "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"

static int g_color_enabled = 1;
static const char* ui(const char* c){ return g_color_enabled ? c : ""; }

#ifdef _WIN32
static void enable_vt_mode(void){
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return;
	DWORD mode = 0;
	if (!GetConsoleMode(hOut, &mode)) return;
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, mode);
}
#endif
	
	/* =========================
	* Utilidades UI
	* ========================= */
	static void ui_clear(void){
		printf("\x1b[2J\x1b[H");
	}
		
		static void ui_pause(void){
			printf("\nPresione ENTER para continuar...");
			fflush(stdout);
			while (getchar() != '\n');
		}
			
			static void ui_banner_inicio(void){
				printf("\n%s========== Sistema %sMedic_L&G%s ==========%s\n",
					   ui(CLR_GREEN), ui(CLR_YELLOW), ui(CLR_GREEN), ui(CLR_RESET));
			}
				
				static void ui_banner(const char* titulo){
					printf("\n%s========== %s ==========%s\n",
						   ui(CLR_GREEN), titulo, ui(CLR_RESET));
				}
					
					static void ui_info(const char* msg){
						printf("%s[INFO]%s %s\n", ui(CLR_BLUE), ui(CLR_RESET), msg);
					}
						
						static void ui_ok(const char* msg){
							printf("%s[OK]%s %s\n", ui(CLR_GREEN), ui(CLR_RESET), msg);
						}
							
							static void ui_error(const char* msg){
								fprintf(stderr, "%s[ERROR]%s %s\n", ui(CLR_RED), ui(CLR_RESET), msg);
							}
								
								/* =========================
								* Entrada
								* ========================= */
								static int read_int(const char* prompt){
									char buf[32];
									for (;;){
										printf("%s", prompt);
										if (!fgets(buf, sizeof(buf), stdin)) continue;
										char* end;
										long v = strtol(buf, &end, 10);
										if (*end == '\n' || *end == '\0') return (int)v;
										ui_error("Entrada inválida.");
									}
								}
									
									static void read_line(const char* prompt, char* out, size_t n){
										printf("%s", prompt);
										fgets(out, (int)n, stdin);
										out[strcspn(out, "\r\n")] = '\0';
									}
										
										/* =========================
										* Menús
										* ========================= */
										static void menu_inicio(void){
											ui_banner_inicio();
											printf("1) Iniciar sesión\n");
											printf("2) Crear usuario\n");
											printf("0) Salir\n");
										}
											
											static void menu_admin(void){
												ui_banner("Administrador");
												printf("1) Agendar cita\n");
												printf("6) Ver citas disponibles\n");
												printf("7) Ver citas con receta\n");
												printf("8) Listar usuarios\n");
												printf("0) Cerrar sesión\n");
											}
												
												static void menu_medico(void){
													ui_banner("Médico");
													printf("1) Ver citas disponibles\n");
													printf("2) Escribir receta\n");
													printf("0) Cerrar sesión\n");
												}
													
													static void menu_paciente(void){
														ui_banner("Paciente");
														printf("1) Agendar cita\n");
														printf("2) Reagendar cita\n");
														printf("3) Ver citas disponibles\n");
														printf("4) Ver mis recetas\n");
														printf("0) Cerrar sesión\n");
													}
														
														/* =========================
														* MAIN
														* ========================= */
														int main(void){
#ifdef _WIN32
															enable_vt_mode();
#endif
															
															Usuarios usuarios;
															usuarios_init(&usuarios);
															usuarios_cargar_txt(RUTA_USUARIOS, &usuarios);
															usuarios_asegurar_admin(&usuarios);
															
															while (1){
																menu_inicio();
																int op = read_int("Opción: ");
																
																if (op == 0){
																	usuarios_guardar_txt(RUTA_USUARIOS, &usuarios);
																	printf("Saliendo del sistema...\n");
																	return 0;
																}
																
																if (op == 1){
																	char user[32], pass[32];
																	ui_banner("Iniciar sesión");
																	read_line("Usuario: ", user, sizeof(user));
																	read_line("Contraseña: ", pass, sizeof(pass));
																	
																	Usuario* actual = usuarios_login(&usuarios, user, pass);
																	if (!actual){
																		ui_error("Credenciales inválidas.");
																		ui_pause();
																		ui_clear();
																		continue;
																	}
																	
																	int logged = 1;
																	while (logged){
																		if (actual->rol == ROL_ADMIN){
																			menu_admin();
																			int r = read_int("Opción: ");
																			switch (r){
																			case 1: paciente_agendar_cita(&usuarios, actual); break;
																			case 6: medico_ver_citas_disponibles(); break;
																			case 7: citas_listar_con_receta(); break;
																			case 8: usuarios_listar_tabla(&usuarios); break;
																			case 0: logged = 0; break;
																			default: ui_error("Opción inválida."); break;
																			}
																		}
																		else if (actual->rol == ROL_MEDICO){
																			menu_medico();
																			int r = read_int("Opción: ");
																			switch (r){
																			case 1: medico_ver_citas_disponibles(); break;
																			case 2: medico_escribir_receta(); break;
																			case 0: logged = 0; break;
																			default: ui_error("Opción inválida."); break;
																			}
																		}
																		else {
																			menu_paciente();
																			int r = read_int("Opción: ");
																			switch (r){
																			case 1: paciente_agendar_cita(&usuarios, actual); break;
																			case 2: paciente_reagendar_cita(); break;
																			case 3: paciente_ver_citas_disponibles(); break;
																			case 4: paciente_ver_mis_recetas(actual); break;
																			case 0: logged = 0; break;
																			default: ui_error("Opción inválida."); break;
																			}
																		}
																		ui_pause();
																		ui_clear();
																	}
																}
																else if (op == 2){
																	Usuario nu;
																	ui_banner("Crear usuario");
																	
																	read_line("Nombre: ", nu.nombre, sizeof(nu.nombre));
																	read_line("Usuario: ", nu.usuario, sizeof(nu.usuario));
																	read_line("Contraseña: ", nu.clave, sizeof(nu.clave));
																	nu.edad = read_int("Edad: ");
																	
																	printf("Tipo:\n1) Paciente\n2) Médico\n");
																	int tipo = read_int("Opción: ");
																	
																	if (tipo == 2){
																		nu.rol = ROL_MEDICO;
																		read_line("Especialidad: ", nu.especialidad, sizeof(nu.especialidad));
																	} else {
																		nu.rol = ROL_PACIENTE;
																		nu.especialidad[0] = '\0';
																	}
																	
																	nu.id = usuarios.count + 1;
																	
																	if (!usuarios_agregar(&usuarios, nu)){
																		ui_error("No se pudo crear el usuario.");
																	} else {
																		usuarios_guardar_txt(RUTA_USUARIOS, &usuarios);
																		ui_ok("Usuario creado.");
																	}
																	ui_pause();
																	ui_clear();
																}
															}
														}
															
