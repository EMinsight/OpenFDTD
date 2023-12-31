/*
dftNear1dZ.cu

DFT of near Z-line
*/

#include "ofd.h"
#include "ofd_cuda.h"
#include "fieldnode.cu"

__global__
static void dftNear1dZ_gpu(
	int commsize, int commrank,
	int nx, int ny, int nz, int imin, int imax,
	int i, int j,
	real_t *ex, real_t *ey, real_t *ez,
	real_t *hx, real_t *hy, real_t *hz,
	d_complex_t *cex, d_complex_t *cey, d_complex_t *cez,
	d_complex_t *chx, d_complex_t *chy, d_complex_t *chz,
	d_complex_t fe, d_complex_t fh)
{
	const int k = threadIdx.x + (blockIdx.x * blockDim.x);
	if (k <= nz) {
		if (((i >= imin) && (i < imax)) || ((commrank == commsize - 1) && (i == nx))) {
			const int id = k       ;
			fieldnode(
				nx, ny, nz, imin, imax,
				i, j, k,
				ex, ey, ez,
				hx, hy, hz,
				&cex[id], &cey[id], &cez[id],
				&chx[id], &chy[id], &chz[id],
				fe, fh, &d_Param);
		}
	}
}

static void dftNear1dZ_cpu(
	int commsize, int commrank,
	int nx, int ny, int nz, int imin, int imax,
	int i, int j,
	real_t *ex, real_t *ey, real_t *ez,
	real_t *hx, real_t *hy, real_t *hz,
	d_complex_t *cex, d_complex_t *cey, d_complex_t *cez,
	d_complex_t *chx, d_complex_t *chy, d_complex_t *chz,
	d_complex_t fe, d_complex_t fh)
{
	for (int k = 0; k <= nz; k++) {
		if (((i >= imin) && (i < imax)) || ((commrank == commsize - 1) && (i == nx))) {
			const int id = k       ;
			fieldnode(
				nx, ny, nz, imin, imax,
				i, j, k,
				ex, ey, ez,
				hx, hy, hz,
				&cex[id], &cey[id], &cez[id],
				&chx[id], &chy[id], &chz[id],
				fe, fh, &h_Param);
		}
	}
}

void dftNear1dZ(int i, int j, int64_t adr1, int64_t adr2,
	d_complex_t *cex, d_complex_t *cey, d_complex_t *cez,
	d_complex_t *chx, d_complex_t *chy, d_complex_t *chz)
{
	if (GPU) {
		cudaMemcpyToSymbol(d_Param, &h_Param, sizeof(param_t));
		dftNear1dZ_gpu<<<CEIL(Nz + 1, near1dBlock), near1dBlock>>>(
			commSize, commRank,
			Nx, Ny, Nz, iMin, iMax,
			i, j,
			Ex, Ey, Ez,
			Hx, Hy, Hz,
			&d_Near1dEx[adr1], &d_Near1dEy[adr1], &d_Near1dEz[adr1],
			&d_Near1dHx[adr1], &d_Near1dHy[adr1], &d_Near1dHz[adr1],
			cEdft[adr2], cHdft[adr2]);
		if (UM) cudaDeviceSynchronize();
	}
	else {
		dftNear1dZ_cpu(
			commSize, commRank,
			Nx, Ny, Nz, iMin, iMax,
			i, j,
			Ex, Ey, Ez,
			Hx, Hy, Hz,
			&cex[adr1], &cey[adr1], &cez[adr1],
			&chx[adr1], &chy[adr1], &chz[adr1],
			cEdft[adr2], cHdft[adr2]);
	}
}
