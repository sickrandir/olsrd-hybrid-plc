#ifndef PLC_DISTRIBUTION_SERVICE_H_
#define PLC_DISTRIBUTION_SERVICE_H_

#endif /* PLC_DISTRIBUTION_SERVICE_H_ */

#define MESSAGE_TYPE    231
#define PARSER_TYPE   MESSAGE_TYPE
#define EMISSION_INTERVAL 3     /* seconds */
#define EMISSION_JITTER         25      /* percent */
#define PLC_VALID_TIME   1800    /* seconds */



int distribution_service_init(void);

