/*
OpenFDTD Version 3.1.1 (CPU + OpenMP)
*/

#define MAIN
#include "ofd.h"
#undef MAIN

#include "ofd_prototype.h"

static void args(int, char *[], int *, int *, int *, int *, char []);

int main(int argc, char *argv[])
{
	const char prog[] = "(CPU+OpenMP)";
	const char errfmt[] = "*** file %s open error.\n";
	char str[BUFSIZ];
	double cpu[4];
	FILE *fp_in = NULL, *fp_out = NULL, *fp_log = NULL;

	const int io = 1;

	// arguments
	runMode = 0;
	int nthread = 1;
	int ev = 0;
	int geom = 0;
	int prompt = 0;
	char fn_in[BUFSIZ] = "";
	args(argc, argv, &nthread, &ev, &geom, &prompt, fn_in);
	//printf("%d %d %d %d %d %s\n", runMode, nthread, ev, geom, prompt, fn_in);

	// set number of threads
#ifdef _OPENMP
	omp_set_num_threads(nthread);
#endif

	// input
	if (io) {
		if ((fp_in = fopen(fn_in, "r")) == NULL) {
			printf(errfmt, fn_in);
			getchar();
			exit(1);
		}
		if (input(fp_in, ((geom != 1) && (runMode < 2)))) {
			getchar();
			exit(1);
		}
		fclose(fp_in);
	}

	// cpu time
	cpu[0] = cputime();

	// === solver ===

	if ((runMode == 0) || (runMode == 1)) {
		// logo
		if (io) {
			sprintf(str, "<<< %s %s Ver.%d.%d.%d >>>", PROGRAM, prog, VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
		}

		if (geom != 1) {
			// open log file
			if (io) {
				if ((fp_log = fopen(FN_log, "w")) == NULL) {
					printf(errfmt, FN_log);
					getchar();
					exit(1);
				}
			}

			// monitor
			if (io) {
				monitor1(fp_log, str);
				sprintf(str, "Thread = %d, Process = 1", nthread);
				monitor1(fp_log, str);
			}
		}
		else {
			if (io) {
				printf("%s\n", str);
				fflush(stdout);
			}
		}

		// plot geometry 3d
		if (io) {
			setup_geom3d();
			if ((geom == 0) || (geom == 1)) {
				plot3dGeom(ev);
			}
		}

		// plot geometry only : exit
		if (geom == 1) {
			exit(0);
		}

		// setup
		setupSize();
		setupSizeNear();
		memalloc1();
		memalloc2();
		memalloc3();
		setup();
		alloc_farfield();

		// monitor
		if (io) {
			monitor2(fp_log, 0);
		}

		// solve
		cpu[1] = cputime();
		solve(io, 1, fp_log);
		cpu[2] = cputime();

		// output
		if (io) {
			// input imepedanece
			if ((NFeed > 0) && (NFreq1 > 0)) {
				zfeed();
				outputZfeed(fp_log);
			}

			// S-parameters
			if ((NPoint > 0) && (NFreq1 > 0)) {
				spara();
				outputSpara(fp_log);
			}

			// coupling
			if ((NFeed > 0) && (NPoint > 0) && (NFreq1 > 0)) {
				outputCoupling(fp_log);
			}

			// cross section
			if (IPlanewave && (NFreq2 > 0)) {
				outputCross(fp_log);
			}

			// output files
			monitor3(fp_log, ev, geom);

			// write ofd.out
			if (runMode == 1) {
				if ((fp_out = fopen(FN_out, "wb")) == NULL) {
					printf(errfmt, FN_out);
					getchar();
					exit(1);
				}
				writeout(fp_out);
				fclose(fp_out);
			}
		}
	}

	// === post ===

	if (io && ((runMode == 0) || (runMode == 2))) {
		// read ofd.out
		if (runMode == 2) {
			if ((fp_out = fopen(FN_out, "rb")) == NULL) {
				printf(errfmt, FN_out);
				getchar();
				exit(1);
			}
			readout(fp_out);
			fclose(fp_out);
			alloc_farfield();
		}

		// post process
		post(ev);
	}

	// free
	if ((runMode == 0) || (runMode == 1)) {
		memfree1();
	}
	memfree3();

	// cpu time
	cpu[3] = cputime();

	if (io && ((runMode == 0) || (runMode == 1))) {
		// cpu time
		monitor4(fp_log, cpu);

		// close log file
		if (fp_log != NULL) {
			fclose(fp_log);
		}
	}

	// prompt
	if (io && prompt) getchar();

	return 0;
}

static void args(int argc, char *argv[],
	int *nthread, int *ev, int *geom, int *prompt, char fn_in[])
{
	const char usage[] = "Usage : ofd [-solver|-post] [-ev] [-geom|-nogeom] [-prompt] <datafile>";

	if (argc < 2) {
		printf("%s\n", usage);
		exit(0);
	}

	while (--argc) {
		++argv;
		if      (!strcmp(*argv, "-solver")) {
			runMode = 1;
		}
		else if (!strcmp(*argv, "-post")) {
			runMode = 2;
		}
		else if (!strcmp(*argv, "-n")) {
			if (--argc) {
				*nthread = atoi(*++argv);
				if (*nthread < 1) *nthread = 1;
			}
			else {
				break;
			}
		}
		else if (!strcmp(*argv, "-ev")) {
			*ev = 1;
		}
		else if (!strcmp(*argv, "-geom")) {
			*geom = 1;
		}
		else if (!strcmp(*argv, "-nogeom")) {
			*geom = 2;
		}
		else if (!strcmp(*argv, "-prompt")) {
			*prompt = 1;
		}
		else if (!strcmp(*argv, "--help")) {
			printf("%s\n", usage);
			exit(0);
		}
		else {
			strcpy(fn_in, *argv);
		}
	}
}
