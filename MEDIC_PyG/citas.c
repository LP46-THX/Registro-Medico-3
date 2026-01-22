
#include "citas.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* =========================
* Estructuras internas
* ========================= */
typedef struct {
	char id[4];          /* "001" .. "999" */
	char paciente[64];
	char medico[64];
	char especialidad[32];
	char fecha[11];      /* DD/MM/AAAA */
	char hora[6];        /* HH:MM */
} CitaTxt;

typedef struct {
	char id[4];
	char paciente[64];
	char medico[64];
	char especialidad[32];
	char fecha[11];
	char hora[6];
	char receta[256];
} RecetaTxt;

/* =========================
* Utilidades privadas
* ========================= */
static int solo_numeros_y_sep(const char *s, char sep){
	if (!s) return 0;
	for (int i=0; s[i]; ++i){
		if (s[i]==sep) continue;
		if (!isdigit((unsigned char)s[i])) return 0;
	}
	return 1;
}
	static int es_bisiesto(int y){
		if (y % 400 == 0) return 1;
		if (y % 100 == 0) return 0;
		return (y % 4 == 0);
	}
		static void trim_nl(char *s){
			if (!s) return;
			s[strcspn(s, "\r\n")] = '\0';
		}
			
			/* Cabeceras si el archivo es nuevo */
			static int asegurar_cabecera(const char *ruta, const char *cab, const char *cols){
				FILE *f = fopen(ruta, "r");
				if (f){ fclose(f); return 0; }
				f = fopen(ruta, "w");
				if (!f) return -1;
				fprintf(f, "%s\n", cab);
				if (cols && cols[0]) fprintf(f, "%s\n", cols);
				fclose(f);
				return 0;
			}
				
				/* Parseo de una línea de cita con '|' */
				static int parse_cita_line(const char *line, CitaTxt *c){
					if (!line || !c) return -1;
					CitaTxt tmp; memset(&tmp, 0, sizeof(tmp));
					int n = sscanf(line, "%3[^|]|%63[^|]|%63[^|]|%31[^|]|%10[^|]|%5[^\n]",
								   tmp.id, tmp.paciente, tmp.medico, tmp.especialidad, tmp.fecha, tmp.hora);
					if (n != 6) return -2;
					*c = tmp;
					return 0;
				}
					
					/* =========================
					* Validaciones públicas
					* ========================= */
					int citas_validar_fecha(const char *f){
						if (!f || strlen(f)!=10) return -1;
						if (f[2]!='/' || f[5]!='/') return -2;
						if (!solo_numeros_y_sep(f, '/')) return -3;
						int d = (f[0]-'0')*10 + (f[1]-'0');
						int m = (f[3]-'0')*10 + (f[4]-'0');
						int y = atoi(f+6);
						if (m < 1 || m > 12) return -4;
						if (d < 1) return -5;
						int dias_mes = 31;
						if (m==4 || m==6 || m==9 || m==11) dias_mes = 30;
						else if (m==2) dias_mes = es_bisiesto(y)?29:28;
						if (d > dias_mes) return -6;
						return 0;
					}
						int citas_validar_hora(const char *h){
							if (!h || strlen(h)!=5) return -1;
							if (h[2] != ':') return -2;
							if (!solo_numeros_y_sep(h, ':')) return -3;
							int hh = (h[0]-'0')*10 + (h[1]-'0');
							int mm = (h[3]-'0')*10 + (h[4]-'0');
							if (hh < 0 || hh > 23) return -4;
							if (mm < 0 || mm > 59) return -5;
							return 0;
						}
							
							/* =========================
							* Interno: generar ID
							* ========================= */
							static void generar_id(char out[4]){
								int max = 0;
								const char *rutas[] = { RUTA_CITAS_DISP, RUTA_CITAS_CONREC };
								for (int r=0; r<2; ++r){
									FILE *f = fopen(rutas[r], "r");
									if (!f) continue;
									char line[512];
									/* saltar cabeceras (# ...) */
									while (fgets(line, sizeof(line), f)){
										if (line[0] != '#') break;
									}
									if (line[0] != '\0'){ /* ya hay 1ra línea de datos en 'line' o quedó vacía */
										do {
											trim_nl(line);
											if (line[0]=='\0') continue;
											CitaTxt c;
											if (parse_cita_line(line, &c)==0){
												int idn = atoi(c.id);
												if (idn > max) max = idn;
											}
										} while (fgets(line, sizeof(line), f));
									}
									fclose(f);
								}
								if (max < 0) max = 0;
								snprintf(out, 4, "%03d", max+1 > 999 ? 999 : max+1);
							}
								
								/* =========================
								* Conflictos
								* ========================= */
								int citas_hay_conflicto(const char *medico, const char *fecha, const char *hora){
									const char *rutas[] = { RUTA_CITAS_DISP, RUTA_CITAS_CONREC };
									for (int r=0; r<2; ++r){
										FILE *f = fopen(rutas[r], "r");
										if (!f) continue;
										char line[512];
										/* saltar cabeceras */
										while (fgets(line, sizeof(line), f)){
											if (line[0] != '#') break;
										}
										if (line[0] != '\0'){
											do {
												trim_nl(line);
												if (line[0]=='\0') continue;
												CitaTxt c;
												if (parse_cita_line(line, &c)==0){
													if (strcmp(c.medico, medico)==0 &&
														strcmp(c.fecha, fecha)==0 &&
														strcmp(c.hora,  hora )==0){
														fclose(f);
														return 1;
													}
												}
											} while (fgets(line, sizeof(line), f));
										}
										fclose(f);
									}
									return 0;
								}
									
									/* =========================
									* Operaciones principales
									* ========================= */
									int citas_agendar(const char *paciente,
													  const char *medico,
													  const char *especialidad,
													  const char *fecha,
													  const char *hora)
									{
										if (citas_validar_fecha(fecha) != 0) return -1;
										if (citas_validar_hora(hora)   != 0) return -2;
										if (citas_hay_conflicto(medico, fecha, hora)) return -3;
										
										if (asegurar_cabecera(RUTA_CITAS_DISP,
															  "# Medic_L&G Citas v1",
															  "# ID|PACIENTE|MEDICO|ESPECIALIDAD|FECHA|HORA") != 0) return -4;
										
										FILE *f = fopen(RUTA_CITAS_DISP, "a");
										if (!f) return -5;
										
										char id[4]; generar_id(id);
										fprintf(f, "%s|%s|%s|%s|%s|%s\n",
												id, paciente, medico, especialidad, fecha, hora);
										fclose(f);
										return 0;
									}
									
									int citas_reagendar(const char *id, const char *nueva_fecha, const char *nueva_hora){
										if (citas_validar_fecha(nueva_fecha) != 0) return -1;
										if (citas_validar_hora(nueva_hora)   != 0) return -2;
										
										FILE *f = fopen(RUTA_CITAS_DISP, "r");
										if (!f) return -3;
										
										FILE *tmp = fopen("citas_tmp.txt", "w");
										if (!tmp){ fclose(f); return -4; }
										
										char line[512];
										int found = 0;
										
										/* Reescribir cabeceras si existen */
										long pos = ftell(f);
										if (fgets(line, sizeof(line), f)){
											if (line[0] == '#'){
												fputs(line, tmp);
												long pos2 = ftell(f);
												if (fgets(line, sizeof(line), f)){
													if (line[0] == '#'){
														fputs(line, tmp);
													} else {
														fseek(f, pos2, SEEK_SET);
													}
												}
											} else {
												fseek(f, pos, SEEK_SET);
											}
										}
										
										while (fgets(line, sizeof(line), f)){
											trim_nl(line);
											if (line[0]=='\0') { fputs("\n", tmp); continue; }
											
											CitaTxt c;
											int ok = parse_cita_line(line, &c);
											if (ok==0 && strcmp(c.id, id)==0){
												/* Validar conflicto con el mismo médico */
												if (citas_hay_conflicto(c.medico, nueva_fecha, nueva_hora)){
													fclose(f); fclose(tmp); remove("citas_tmp.txt"); return -6;
												}
												strncpy(c.fecha, nueva_fecha, sizeof(c.fecha)-1);
												strncpy(c.hora,  nueva_hora,  sizeof(c.hora)-1);
												fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
														c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
												found = 1;
											} else if (ok==0){
												fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
														c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
											} else {
												/* Si no parsea, preservar línea tal cual para no perder datos */
												fprintf(tmp, "%s\n", line);
											}
										}
										
										fclose(f); fclose(tmp);
										if (!found){ remove("citas_tmp.txt"); return -5; }
										
										remove(RUTA_CITAS_DISP);
										rename("citas_tmp.txt", RUTA_CITAS_DISP);
										return 0;
									}
										
										/* =========================
										* Eliminaciones
										* ========================= */
										int citas_borrar_por_id(const char *id){
											FILE *f = fopen(RUTA_CITAS_DISP, "r");
											if (!f) return -1;
											
											FILE *tmp = fopen("citas_tmp.txt", "w");
											if (!tmp){ fclose(f); return -2; }
											
											char line[512];
											int eliminado = 0;
											
											/* Copiar cabeceras */
											long pos = ftell(f);
											if (fgets(line, sizeof(line), f)){
												if (line[0]=='#'){
													fputs(line, tmp);
													long pos2 = ftell(f);
													if (fgets(line, sizeof(line), f)){
														if (line[0]=='#') fputs(line, tmp);
														else fseek(f, pos2, SEEK_SET);
													}
												} else fseek(f, pos, SEEK_SET);
											}
											
											while (fgets(line, sizeof(line), f)){
												trim_nl(line);
												if (line[0]=='\0'){ fputs("\n", tmp); continue; }
												CitaTxt c;
												if (parse_cita_line(line, &c)==0){
													if (strcmp(c.id, id)==0){ eliminado = 1; continue; }
													fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
															c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
												} else {
													fprintf(tmp, "%s\n", line);
												}
											}
											
											fclose(f); fclose(tmp);
											remove(RUTA_CITAS_DISP);
											rename("citas_tmp.txt", RUTA_CITAS_DISP);
											return eliminado ? 0 : -3;
										}
											
											int citas_borrar_por_medico(const char *medico){
												FILE *f = fopen(RUTA_CITAS_DISP, "r");
												if (!f) return -1;
												
												FILE *tmp = fopen("citas_tmp.txt", "w");
												if (!tmp){ fclose(f); return -2; }
												
												char line[512];
												int borradas = 0;
												
												/* Copiar cabeceras */
												long pos = ftell(f);
												if (fgets(line, sizeof(line), f)){
													if (line[0]=='#'){
														fputs(line, tmp);
														long pos2 = ftell(f);
														if (fgets(line, sizeof(line), f)){
															if (line[0]=='#') fputs(line, tmp);
															else fseek(f, pos2, SEEK_SET);
														}
													} else fseek(f, pos, SEEK_SET);
												}
												
												while (fgets(line, sizeof(line), f)){
													trim_nl(line);
													if (line[0]=='\0'){ fputs("\n", tmp); continue; }
													CitaTxt c;
													if (parse_cita_line(line, &c)==0){
														if (strcmp(c.medico, medico)==0){ borradas++; continue; }
														fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
																c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
													} else {
														fprintf(tmp, "%s\n", line);
													}
												}
												
												fclose(f); fclose(tmp);
												remove(RUTA_CITAS_DISP);
												rename("citas_tmp.txt", RUTA_CITAS_DISP);
												return borradas > 0 ? 0 : -3;
											}
												
												int citas_borrar_por_paciente(const char *paciente){
													FILE *f = fopen(RUTA_CITAS_DISP, "r");
													if (!f) return -1;
													
													FILE *tmp = fopen("citas_tmp.txt", "w");
													if (!tmp){ fclose(f); return -2; }
													
													char line[512];
													int borradas = 0;
													
													/* Copiar cabeceras */
													long pos = ftell(f);
													if (fgets(line, sizeof(line), f)){
														if (line[0]=='#'){
															fputs(line, tmp);
															long pos2 = ftell(f);
															if (fgets(line, sizeof(line), f)){
																if (line[0]=='#') fputs(line, tmp);
																else fseek(f, pos2, SEEK_SET);
															}
														} else fseek(f, pos, SEEK_SET);
													}
													
													while (fgets(line, sizeof(line), f)){
														trim_nl(line);
														if (line[0]=='\0'){ fputs("\n", tmp); continue; }
														CitaTxt c;
														if (parse_cita_line(line, &c)==0){
															if (strcmp(c.paciente, paciente)==0){ borradas++; continue; }
															fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
																	c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
														} else {
															fprintf(tmp, "%s\n", line);
														}
													}
													
													fclose(f); fclose(tmp);
													remove(RUTA_CITAS_DISP);
													rename("citas_tmp.txt", RUTA_CITAS_DISP);
													return borradas > 0 ? 0 : -3;
												}
													
													/* =========================
													* Recetas
													* ========================= */
													int citas_crear_receta(const char *id, const char *texto){
														FILE *f = fopen(RUTA_CITAS_DISP, "r");
														if (!f) return -1;
														
														FILE *fcon = fopen(RUTA_CITAS_CONREC, "a");
														if (!fcon){ fclose(f); return -2; }
														
														/* Si el archivo de 'con receta' no existe, se pone cabecera nueva de citas */
														asegurar_cabecera(RUTA_CITAS_CONREC,
																		  "# Medic_L&G Citas v1",
																		  "# ID|PACIENTE|MEDICO|ESPECIALIDAD|FECHA|HORA");
														
														char line[512];
														CitaTxt target; int found = 0;
														
														/* Copiar cabeceras de DISP (si existen) a un tmp y omitir el registro movido */
														FILE *tmp = fopen("citas_tmp.txt", "w");
														if (!tmp){ fclose(f); fclose(fcon); return -4; }
														
														/* Copiar cabeceras de DISP */
														long pos = ftell(f);
														if (fgets(line, sizeof(line), f)){
															if (line[0]=='#'){
																fputs(line, tmp);
																long pos2 = ftell(f);
																if (fgets(line, sizeof(line), f)){
																	if (line[0]=='#') fputs(line, tmp);
																	else fseek(f, pos2, SEEK_SET);
																}
															} else fseek(f, pos, SEEK_SET);
														}
														
														while (fgets(line, sizeof(line), f)){
															trim_nl(line);
															if (line[0]=='\0'){ fputs("\n", tmp); continue; }
															CitaTxt c;
															int ok = parse_cita_line(line, &c);
															if (ok==0 && strcmp(c.id, id)==0){
																/* mover a CONREC */
																fprintf(fcon, "%s|%s|%s|%s|%s|%s\n",
																		c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
																target = c; found = 1;
																/* No se escribe en tmp */
															} else if (ok==0){
																fprintf(tmp, "%s|%s|%s|%s|%s|%s\n",
																		c.id,c.paciente,c.medico,c.especialidad,c.fecha,c.hora);
															} else {
																fprintf(tmp, "%s\n", line);
															}
														}
														
														fclose(f); fclose(fcon); fclose(tmp);
														
														if (!found){ remove("citas_tmp.txt"); return -3; }
														
														remove(RUTA_CITAS_DISP);
														rename("citas_tmp.txt", RUTA_CITAS_DISP);
														
														/* Registrar texto de receta en recetas.txt (formato original simple: libre) */
														FILE *frx = fopen(RUTA_RECETAS, "a");
														if (!frx) return -5;
														fprintf(frx,
																"%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
																target.id, target.paciente, target.medico, target.especialidad,
																target.fecha, target.hora, (texto?texto:""));
														fclose(frx);
														
														return 0;
													}
														
														/* =========================
														* Listados simples (raw)
														* ========================= */
														void citas_listar_disponibles(void){
															FILE *f = fopen(RUTA_CITAS_DISP, "r");
															if (!f) return;
															char line[512];
															while (fgets(line, sizeof(line), f)) printf("%s", line);
															fclose(f);
														}
															void citas_listar_con_receta(void){
																FILE *f = fopen(RUTA_CITAS_CONREC, "r");
																if (!f) return;
																char line[512];
																while (fgets(line, sizeof(line), f)) printf("%s", line);
																fclose(f);
															}
																
																
																/* =========================
																* Listar recetas por paciente
																* ========================= */
																void recetas_listar_por_paciente(const char *paciente)
																{
																	if (!paciente) return;
																	
																	FILE *f = fopen(RUTA_RECETAS, "r");
																	if (!f) return;
																	
																	char line[1024];
																	
																	while (fgets(line, sizeof(line), f)) {
																		line[strcspn(line, "\r\n")] = 0;
																		
																		char id[8], pac[64], med[64], esp[32], fecha[16], hora[16], receta[512];
																		
																		strncpy(id, line, sizeof(id)-1);
																		if (!fgets(pac, sizeof(pac), f)) break;
																		if (!fgets(med, sizeof(med), f)) break;
																		if (!fgets(esp, sizeof(esp), f)) break;
																		if (!fgets(fecha, sizeof(fecha), f)) break;
																		if (!fgets(hora, sizeof(hora), f)) break;
																		if (!fgets(receta, sizeof(receta), f)) break;
																		
																		pac[strcspn(pac, "\r\n")] = 0;
																		med[strcspn(med, "\r\n")] = 0;
																		esp[strcspn(esp, "\r\n")] = 0;
																		fecha[strcspn(fecha, "\r\n")] = 0;
																		hora[strcspn(hora, "\r\n")] = 0;
																		receta[strcspn(receta, "\r\n")] = 0;
																		
																		if (strcmp(pac, paciente) == 0) {
																			printf("\n--------------------------\n");
																			printf("ID: %s\n", id);
																			printf("Medico: %s\n", med);
																			printf("Especialidad: %s\n", esp);
																			printf("Fecha: %s\n", fecha);
																			printf("Hora: %s\n", hora);
																			printf("Receta: %s\n", receta);
																		}
																	}
																	
																	fclose(f);
																}
																
