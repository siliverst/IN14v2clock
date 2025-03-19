#ifndef _SERVICE_TIMING_H_
#define _SERVICE_TIMING_H_

#ifdef	__cplusplus
extern "C" {
#endif


void service_timing_init (void);
void service_timing_10ms_proc (void);
void set_service_timing_mutex (void);
void reset_service_timing_mutex (void);


#ifdef	__cplusplus
}
#endif

#endif /* _SERVICE_TIMING_H_ */
