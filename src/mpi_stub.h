#ifndef MPI_STUB_H
#define MPI_STUB_H
#include <cstring>
#include <cstdlib>

using MPI_Comm = int;
using MPI_Status = int;

#define MPI_DOUBLE 0
#define MPI_SUM 0
#define MPI_MAX 0
#define MPI_COMM_WORLD 0

inline int MPI_Init(int* argc, char*** argv) { return 0; }
inline int MPI_Comm_rank(MPI_Comm, int* rank) { *rank = 0; return 0; }
inline int MPI_Comm_size(MPI_Comm, int* size) { *size = 1; return 0; }
inline int MPI_Allreduce(const void* sendbuf, void* recvbuf, int count, int,
                         int, MPI_Comm) {
    std::memcpy(recvbuf, sendbuf, sizeof(double) * count);
    return 0;
}
inline int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, int,
                      int, int, MPI_Comm) {
    if (recvbuf != sendbuf) std::memcpy(recvbuf, sendbuf, sizeof(double) * count);
    return 0;
}
inline int MPI_Sendrecv(const void* sbuf, int scount, int, int, int,
                        void* rbuf, int rcount, int, int, int, MPI_Comm,
                        MPI_Status*) {
    std::memcpy(rbuf, sbuf, sizeof(double) * scount);
    return 0;
}
inline int MPI_Finalize() { return 0; }

#endif // MPI_STUB_H
