#include <chrono>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

std::vector<int> read_file(){
    std::fstream fs("C:\\Users\\Lap\\Downloads\\datos.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;
    while (std::getline(fs, line)){
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

std::vector<int> aplanar_tabla_frecuencias(const std::map<int, int>& map){
    std::vector<int> respuesta;
    for (auto par : map){
        respuesta.push_back(par.first);
        respuesta.push_back(par.second);
    }
    return respuesta;
}

std::map<int, int> desaplanar_tabla_frecuencias(const std::vector<int>& vector){
    std::map<int, int> respuesta;
    for (int i = 0; i < vector.size(); i += 2){
        respuesta[vector[i]] = vector[i + 1];
    }
    return respuesta;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> vector = read_file();
    std::map<int, int> tabla_frecuencias;

    int num_elementos, resto;
    std::vector<int> vector_elementos;
    std::vector<int> vector_desplazamientos;
    std::vector<int> vector_elementos_respuesta;
    std::vector<int> vector_desplazamientos_respuesta;
    std::vector<int> vector_respuestas;
    std::vector<int> vector_local;
    std::map<int, int> tabla_frecuencias_local;

    if (rank == 0){
        /*for (int i = 0; i < 1000; i++){
            printf("%d\n", i);
        }*/
        num_elementos = vector.size() / nprocs;
        resto = vector.size() % nprocs;

        for (int i = 0; i < nprocs; i++){
            vector_elementos[i] = (rank == nprocs - 1) ? num_elementos + resto : num_elementos;
        }
        vector_desplazamientos[0] = 0;
        for (int i = 1; i < nprocs; i++){
            vector_desplazamientos[i] = i * num_elementos;
        }
    }

    MPI_Scatterv(vector.data(), vector_elementos.data(), vector_desplazamientos.data(), MPI_INT,
                 vector_local.data(), vector_elementos[rank], MPI_INT,
                 0, MPI_COMM_WORLD);


    for (int numero : vector_local){
        tabla_frecuencias_local[numero]++;
    }
    printf("Rank %d\n", rank);
    printf("Valor | Frecuencia\n");
    for (auto par : tabla_frecuencias_local){
        printf("| %3d | %6d|\n", par.first, par.second);
    }

    std::vector<int> tabla_frec_local_aplanada = aplanar_tabla_frecuencias(tabla_frecuencias_local);
    int tamanio_tabla_frec_local_aplanada = tabla_frec_local_aplanada.size();

    MPI_Send(&tamanio_tabla_frec_local_aplanada,1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    if (rank == 0){
        for (int i = 0; i < nprocs; i++){
            int valor;
            MPI_Recv(&valor, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector_elementos_respuesta[i] = valor;
        }
        vector_desplazamientos_respuesta[0] = 0;
        for (int i = 1; i < nprocs; i++){
            vector_desplazamientos_respuesta[i] = vector_desplazamientos_respuesta[i - 1] + vector_elementos_respuesta[i - 1];
        }
    }

    MPI_Gatherv(tabla_frec_local_aplanada.data(), tamanio_tabla_frec_local_aplanada, MPI_INT,
                vector_respuestas.data(), vector_elementos_respuesta.data(), vector_desplazamientos_respuesta.data(), MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0){
        tabla_frecuencias = desaplanar_tabla_frecuencias(vector_respuestas);
        printf("Valor | Frecuencia\n");
        for (auto par : tabla_frecuencias){
            printf("| %3d | %6d|\n", par.first, par.second);
        }
    }

    MPI_Finalize();
    return 0;
}
