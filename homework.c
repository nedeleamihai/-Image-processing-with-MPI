#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int width;
int height;
int NrFiltre;
int rank;

void AplicaFiltre(unsigned char matrix[][width], int NrLiniiProces, const char *filtre[]){
	float smooth[3][3] = {{1, 1, 1},{1, 1, 1},{1, 1, 1}};
	float blur[3][3] = {{1, 2, 1},{2, 4, 2},{1, 2, 1}};
	float sharpen[3][3] = {{0, -2, 0},{-2, 11, -2},{0, -2, 0}};
	float mean[3][3] = {{-1, -1, -1},{-1, 9, -1},{-1, -1, -1}};
	float emboss[3][3] = {{0, 1, 0},{0, 0, 0},{0, -1, 0}};
	int i, j, f;
		
	for(f = 0; f < NrFiltre; f++){
			unsigned char matrixAuxiliar[NrLiniiProces + 2][width];
			memcpy(matrixAuxiliar, matrix, (NrLiniiProces + 2) * width * sizeof(unsigned char));
			
			for(i = 1; i < NrLiniiProces + 1; i++){
				for(j = 1; j < width - 1; j++){
					float filtru[3][3];
					float dimensiune;
										
					if(strcmp(filtre[f], "smooth") == 0){
						memcpy(filtru, smooth, 9 * sizeof(float));
						dimensiune = 9.0f;								
					}
					
					if(strcmp(filtre[f], "blur") == 0){
						memcpy(filtru, blur, 9 * sizeof(float));
						dimensiune = 16.0f;         															
					}
					
					if(strcmp(filtre[f], "sharpen") == 0){
						memcpy(filtru, sharpen, 9 * sizeof(float));
						dimensiune = 3.0f;             															
					}
					
					if(strcmp(filtre[f], "mean") == 0){
						memcpy(filtru, mean, 9 * sizeof(float));
						dimensiune = 1.0f;             															
					}
					
					if(strcmp(filtre[f], "emboss") == 0){
						memcpy(filtru, emboss, 9 * sizeof(float));
						dimensiune = 1.0f;             															
					}
					
					float s = 0.0f;
					int l, c, cs, ls = 0;
						
					for(l = i - 1; l <= i + 1; l++){
						cs = 0;
						for(c = j - 1; c <= j + 1; c++){
							float xxx = (float)matrix[l][c] * filtru[ls][cs];
							s += xxx * (1 / dimensiune);
							cs++;
						}
						ls++;
					}
					matrixAuxiliar[i][j] = (unsigned char)s;
				}
			}
			memcpy(matrix, matrixAuxiliar, (NrLiniiProces + 2) * width * sizeof(unsigned char));
	}		
}

int main(int argc, char **argv){
	char color[3];
	int LungimeImagine, maxval, numcolors = 1;    	
	int i, j, p;
    	
    	//Citeste filtrele din linia de comanda
    	NrFiltre = argc - 3;
    	const char *filtre[NrFiltre];    	
	for(i = 0; i < NrFiltre; i++){
		filtre[i] = argv[i + 3];
	}
    	
	//Citeste din imagine
	FILE *file = fopen(argv[1], "rb");
	fscanf(file, "%s", color);
	fscanf(file, "%d", &width);
	fscanf(file, "%d", &height);
	fscanf(file, "%d\n", &maxval);
	if(strcmp(color, "P6") == 0){
		numcolors = 3;
	}
	LungimeImagine = width * height * numcolors;
	
	//Citeste matricea din imagine	
    	unsigned char greyscale[height][width];
    	unsigned char red[height][width];
    	unsigned char green[height][width];
    	unsigned char blue[height][width];
	unsigned char *buffer = NULL;
	buffer = (unsigned char *)malloc(sizeof(unsigned char) * LungimeImagine);	
	fread(buffer, sizeof(unsigned char), LungimeImagine, file);
	
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			if(numcolors == 1){
				greyscale[i][j] = buffer[i * width + j];
			}else{
				red[i][j] = buffer[3 * i * width + j * 3];
				green[i][j] = buffer[3 * i * width + j * 3 + 1];
				blue[i][j] = buffer[3 * i * width + j * 3 + 2];
			}
		}
	}
		
	int size, tag = 0;
	MPI_Init(&argc, &argv);
    	MPI_Comm_size(MPI_COMM_WORLD, &size);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	
    	//Vom seta numarul de linii pe care sa l prelucreze fiecare proces
    	int NrTotalLinii = height - 2;
    	int NrTotalProcese = size;
    	int nrLinii;
    	int LiniiProcese[size];    	
    	for(p = 0; p < size; p++){
    		nrLinii = ceil((float)NrTotalLinii / NrTotalProcese);
    		LiniiProcese[p] = nrLinii;
    		NrTotalLinii -= nrLinii;
    		NrTotalProcese--;
    	}	
    	
    	if(rank == 0){
		int lp = 0;
		for(p = 1; p < size; p++){
			unsigned char greyscale2[LiniiProcese[p] + 2][width];
			unsigned char red2[LiniiProcese[p] + 2][width];
			unsigned char green2[LiniiProcese[p] + 2][width];
			unsigned char blue2[LiniiProcese[p] + 2][width];
			lp += LiniiProcese[p - 1];
					
			for(i = 0; i < LiniiProcese[p] + 2; i++){
				for(j = 0; j < width; j++){
					if(numcolors == 1){
						greyscale2[i][j] = greyscale[lp + i][j];
					}else{
						red2[i][j] = red[lp + i][j];
						green2[i][j] = green[lp + i][j];
						blue2[i][j] = blue[lp + i][j];
					}
				}
			}
			if(numcolors == 1){
				MPI_Send(&greyscale2, (LiniiProcese[p] + 2) * width, MPI_CHAR, p, tag, MPI_COMM_WORLD);
			}else{
				MPI_Send(&red2, (LiniiProcese[p] + 2) * width, MPI_CHAR, p, tag, MPI_COMM_WORLD);
				MPI_Send(&green2, (LiniiProcese[p] + 2) * width, MPI_CHAR, p, tag, MPI_COMM_WORLD);
				MPI_Send(&blue2, (LiniiProcese[p] + 2) * width, MPI_CHAR, p, tag, MPI_COMM_WORLD);
			}			
		}
		
		//Procesul 0 aplica filtrele pe linii
		unsigned char greyscale2[LiniiProcese[0] + 2][width];
		unsigned char red2[LiniiProcese[0] + 2][width];
		unsigned char green2[LiniiProcese[0] + 2][width];
		unsigned char blue2[LiniiProcese[0] + 2][width];
		
		if(numcolors == 1){
			memcpy(greyscale2, greyscale, (LiniiProcese[0] + 2) * width * sizeof(unsigned char));
			AplicaFiltre(greyscale2, LiniiProcese[0], filtre);
		}else{
			memcpy(red2, red, (LiniiProcese[0] + 2) * width * sizeof(unsigned char));
			memcpy(green2, green, (LiniiProcese[0] + 2) * width * sizeof(unsigned char));
			memcpy(blue2, blue, (LiniiProcese[0] + 2) * width * sizeof(unsigned char));
			
			AplicaFiltre(red2, LiniiProcese[0], filtre);
			AplicaFiltre(green2, LiniiProcese[0], filtre);
			AplicaFiltre(blue2, LiniiProcese[0], filtre);
		}
		
		for(i = 1; i < LiniiProcese[0] + 1; i++){
			for(j = 1; j < width - 1; j++){
				if(numcolors == 1){
					greyscale[i][j] = greyscale2[i][j];
				}else{
					red[i][j] = red2[i][j];
					green[i][j] = green2[i][j];
					blue[i][j] = blue2[i][j];
				}				
			}
		}
		
	}else{
		unsigned char greyscale2[LiniiProcese[rank] + 2][width];
		unsigned char red2[LiniiProcese[rank] + 2][width];
		unsigned char green2[LiniiProcese[rank] + 2][width];
		unsigned char blue2[LiniiProcese[rank] + 2][width];
		
		if(numcolors == 1){
			MPI_Recv( &greyscale2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );	
			AplicaFiltre(greyscale2, LiniiProcese[rank], filtre);	
			MPI_Send(&greyscale2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
		}else{
			MPI_Recv( &red2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			MPI_Recv( &green2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			MPI_Recv( &blue2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			
			AplicaFiltre(red2, LiniiProcese[rank], filtre);
			AplicaFiltre(green2, LiniiProcese[rank], filtre);
			AplicaFiltre(blue2, LiniiProcese[rank], filtre);
	
			MPI_Send(&red2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, tag, MPI_COMM_WORLD);			
			MPI_Send(&green2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
			MPI_Send(&blue2[0][0], (LiniiProcese[rank] + 2) * width, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
		}
	}
	
	if(rank == 0){
		int lp = 0;
		for(p = 1; p < size; p++){
			unsigned char greyscale2[LiniiProcese[p] + 2][width];
			unsigned char red2[LiniiProcese[p] + 2][width];
			unsigned char green2[LiniiProcese[p] + 2][width];
			unsigned char blue2[LiniiProcese[p] + 2][width];
			
			lp += LiniiProcese[p - 1];
			
			if(numcolors == 1){
				MPI_Recv(&greyscale2[0][0], (LiniiProcese[p] + 2) * width, MPI_CHAR, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			}else{
				MPI_Recv(&red2[0][0], (LiniiProcese[p] + 2) * width, MPI_CHAR, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv(&green2[0][0], (LiniiProcese[p] + 2) * width, MPI_CHAR, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv(&blue2[0][0], (LiniiProcese[p] + 2) * width, MPI_CHAR, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			}		
			
		printf("Rank %d ultima linie\n", p);
		for(j = 0; j < width; j++){
			printf("%u ", greyscale2[LiniiProcese[p]][j]);
		}
		printf("\n\n");	
		
		printf("Rank %d prima linie\n", p);
		for(j = 0; j < width; j++){
			printf("%u ", greyscale2[0][j]);
		}
		printf("\n\n");
			
			for(i = 1; i < LiniiProcese[p] + 1; i++){
				for(j = 1; j < width - 1; j++){
					if(numcolors == 1){
						greyscale[lp + i][j] = greyscale2[i][j];
					}else{
						red[lp + i][j] = red2[i][j];
						green[lp + i][j] = green2[i][j];
						blue[lp + i][j] = blue2[i][j];
					}
				}
			}
		}
		
		FILE *out = fopen(argv[2], "wb");
    		fprintf (out, "%s\n", color);
    		fprintf (out, "%d %d\n", width, height);
    		fprintf (out, "%d\n", maxval);
		
		if(numcolors == 1){
			fwrite(greyscale, LungimeImagine * sizeof(unsigned char), 1, out);
		}else{
			unsigned char *output = NULL;
			output = (unsigned char *)malloc(sizeof(unsigned char) * LungimeImagine);
			
			for (i = 0; i < height; i++){
				for (j = 0; j < width; j++){
					output[3 * i * width + j * 3] = red[i][j];
					output[3 * i * width + j * 3 + 1] = green[i][j];
					output[3 * i * width + j * 3 + 2] = blue[i][j];
				}
			}
			
			fwrite(output, LungimeImagine * sizeof(unsigned char), 1, out);
		}
    		fclose(out);
	}
	
    	MPI_Finalize();
    	return 0;
}
