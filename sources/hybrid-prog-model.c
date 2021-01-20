#include <pmix.h>

//<EG BEGIN ID="declare_model_cb">
static void model_declared_cb(size_t evhdlr_registration_id,
                              pmix_status_t status, const pmix_proc_t *source,
                              pmix_info_t info[], size_t ninfo,
                              pmix_info_t results[], size_t nresults,
                              pmix_event_notification_cbfunc_fn_t cbfunc,
                              void *cbdata) {
  int n;
  for (n = 0; n < ninfo; n++) {
    if (PMIX_CHECK_KEY(&info[n], PMIX_PROGRAMMING_MODEL) &&
        strcmp(info[n].value.data.string, "MPI") == 0) {
      /* ignore our own declaration */
      break;
    } else {
      /* actions to perform when another model registers */
    }
  }
  if (NULL != cbfunc) {
    /* tell the event handler that we are only a partial step */
    cbfunc(PMIX_EVENT_PARTIAL_ACTION_TAKEN, NULL, 0, NULL, NULL, cbdata);
  }
}
//<EG END ID="declare_model_cb">

//<EG BEGIN ID="omp_thread">
static void parallel_region_OMP_cb(size_t evhdlr_registration_id,
                                   pmix_status_t status,
                                   const pmix_proc_t *source,
                                   pmix_info_t info[], size_t ninfo,
                                   pmix_info_t results[], size_t nresults,
                                   pmix_event_notification_cbfunc_fn_t cbfunc,
                                   void *cbdata) {
  /* do what we need OpenMP to do on entering a parallel region */
  if (NULL != cbfunc) {
    /* tell the event handler that we are only a partial step */
    cbfunc(PMIX_EVENT_PARTIAL_ACTION_TAKEN, NULL, 0, NULL, NULL, cbdata);
  }
}
//<EG END ID="omp_thread">

//<EG BEGIN ID="mpi_thread">
static void parallel_region_MPI_cb(size_t evhdlr_registration_id,
                                   pmix_status_t status,
                                   const pmix_proc_t *source,
                                   pmix_info_t info[], size_t ninfo,
                                   pmix_info_t results[], size_t nresults,
                                   pmix_event_notification_cbfunc_fn_t cbfunc,
                                   void *cbdata) {
  /* do what we need MPI to do on entering a parallel region */
  if (NULL != cbfunc) {
    /* do what we need MPI to do on entering a parallel region */
    cbfunc(PMIX_EVENT_ACTION_COMPLETE, NULL, 0, NULL, NULL, cbdata);
  }
}
//<EG END ID="mpi_thread">

static int openmp_handler() {
  pmix_info_t *info;
  int rc;

  bool is_true = true;
  //<EG BEGIN ID="omp_thread">
  pmix_status_t code = PMIX_OPENMP_PARALLEL_ENTERED;
  PMIX_INFO_CREATE(info, 2);
  PMIX_INFO_LOAD(&info[0], PMIX_EVENT_HDLR_NAME, "OpenMP-Master", PMIX_STRING);
  PMIX_INFO_LOAD(&info[1], PMIX_EVENT_HDLR_FIRST, &is_true, PMIX_BOOL);
  rc = PMIx_Register_event_handler(&code, 1, info, 1, parallel_region_OMP_cb, NULL, NULL);
  PMIX_INFO_FREE(info, 2);
  //<EG END ID="omp_thread">

  return rc;
}

static int mpi_handler() {
  pmix_info_t *info;
  int rc;

  //<EG BEGIN ID="mpi_thread">
  pmix_status_t code = PMIX_OPENMP_PARALLEL_ENTERED;
  PMIX_INFO_CREATE(info, 2);
  PMIX_INFO_LOAD(&info[0], PMIX_EVENT_HDLR_NAME, "MPI-Thread", PMIX_STRING);
  PMIX_INFO_LOAD(&info[1], PMIX_EVENT_HDLR_AFTER, "OpenMP-Master", PMIX_STRING);
  rc = PMIx_Register_event_handler(&code, 1, info, 1, parallel_region_MPI_cb, NULL, NULL);
  PMIX_INFO_FREE(info, 2);
  //<EG END ID="mpi_thread">

  return rc;
}

int main() {
  //<EG BEGIN ID="declare_model">
  pmix_proc_t myproc;
  pmix_info_t *info;

  PMIX_INFO_CREATE(info, 4);
  PMIX_INFO_LOAD(&info[0], PMIX_PROGRAMMING_MODEL, "MPI", PMIX_STRING);
  PMIX_INFO_LOAD(&info[1], PMIX_MODEL_LIBRARY_NAME, "FooMPI", PMIX_STRING);
  PMIX_INFO_LOAD(&info[2], PMIX_MODEL_LIBRARY_VERSION, "1.0.0", PMIX_STRING);
  PMIX_INFO_LOAD(&info[3], PMIX_THREADING_MODEL, "pthread", PMIX_STRING);
  pmix_status_t rc = PMIx_Init(&myproc, info, 4);
  PMIX_INFO_FREE(info, 4);
  //<EG END ID="declare_model">
  //<EG BEGIN ID="declare_model_cb">

  pmix_status_t code = PMIX_MODEL_DECLARED;
  rc = PMIx_Register_event_handler(&code, 1, NULL, 0, model_declared_cb, NULL, NULL);
  //<EG END ID="declare_model_cb">

  openmp_handler();
  mpi_handler();

  //<EG BEGIN ID="notify_event">
  PMIx_Notify_event(PMIX_OPENMP_PARALLEL_ENTERED, &myproc, PMIX_RANGE_LOCAL, NULL, 0, NULL, NULL);
  //<EG END ID="notify_event">
}
