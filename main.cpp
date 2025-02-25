#include <iostream>
#include <mpi.h>
#include <fstream>
#include <map>
#include <vector>
#include <string>

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
    for (size_t i = 0; i < vector.size(); i += 2){
        respuesta[vector[i]] = vector[i + 1];
    }
    return respuesta;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> vector;
    if (rank == 0) {
        vector = read_file();
    }

    int num_elementos, resto;
    std::vector<int> vector_elementos(nprocs, 0);
    std::vector<int> vector_desplazamientos(nprocs, 0);

    if (rank == 0){
        num_elementos = vector.size() / nprocs;
        resto = vector.size() % nprocs;

        for (int i = 0; i < nprocs; i++){
            vector_elementos[i] = (i == nprocs - 1) ? num_elementos + resto : num_elementos;
        }
        for (int i = 1; i < nprocs; i++){
            vector_desplazamientos[i] = vector_desplazamientos[i - 1] + vector_elementos[i - 1];
        }
    }

    // Broadcast para que todos los procesos conozcan cuántos elementos recibirán
    MPI_Bcast(vector_elementos.data(), nprocs, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> vector_local(vector_elementos[rank]);

    // Distribuir los datos a cada proceso
    MPI_Scatterv(vector.data(), vector_elementos.data(), vector_desplazamientos.data(), MPI_INT,
                 vector_local.data(), vector_elementos[rank], MPI_INT, 0, MPI_COMM_WORLD);

    std::map<int, int> tabla_frecuencias_local;
    for (int numero : vector_local){
        tabla_frecuencias_local[numero]++;
    }

    std::vector<int> tabla_frec_local_aplanada = aplanar_tabla_frecuencias(tabla_frecuencias_local);
    int tamanio_local = tabla_frec_local_aplanada.size();

    std::vector<int> vector_elementos_respuesta(nprocs, 0);
    std::vector<int> vector_desplazamientos_respuesta(nprocs, 0);
    std::vector<int> vector_respuestas;

    MPI_Gather(&tamanio_local, 1, MPI_INT, vector_elementos_respuesta.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0){
        vector_desplazamientos_respuesta[0] = 0;
        for (int i = 1; i < nprocs; i++){
            vector_desplazamientos_respuesta[i] = vector_desplazamientos_respuesta[i - 1] + vector_elementos_respuesta[i - 1];
        }
        int total_size = vector_desplazamientos_respuesta[nprocs - 1] + vector_elementos_respuesta[nprocs - 1];
        vector_respuestas.resize(total_size);
    }

    MPI_Gatherv(tabla_frec_local_aplanada.data(), tamanio_local, MPI_INT,
                vector_respuestas.data(), vector_elementos_respuesta.data(),
                vector_desplazamientos_respuesta.data(), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0){
        std::map<int, int> tabla_frecuencias = desaplanar_tabla_frecuencias(vector_respuestas);
        printf("Valor | Frecuencia\n");
        for (auto par : tabla_frecuencias){
            printf("| %3d | %6d|\n", par.first, par.second);
        }
    }

    MPI_Finalize();
    return 0;
}
