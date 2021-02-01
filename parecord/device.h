#ifndef PAR_DEVICE_H
#define PAR_DEVICE_H

struct list_devices {
	char *name;
	char *description;
};

struct list_devices *getInputDeviceList(int *len);
void freeDeviceList(struct list_devices *list, int len);

#endif /* PAR_DEVICE_H */
