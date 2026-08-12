#ifndef __TOUCHPANEL_H
#define __TOUCHPANEL_H

/**
 * struct touchpanel_priv - information about a touchpanel, for the uclass
 *
 * @size_x:	horizontal resolution as seen by users of this uclass, that is
 *		after the orientation below has been applied
 * @size_y:	vertical resolution as seen by users of this uclass
 * @max_x:	largest x the digitizer itself reports, -ENODATA if unknown
 * @max_y:	largest y the digitizer itself reports, -ENODATA if unknown
 * @inverted_x:	report x mirrored about the centre of the panel
 * @inverted_y:	report y mirrored about the centre of the panel
 * @swapped_x_y: report x and y the other way round, for a digitizer that is
 *		mounted at right angles to the display
 */
struct touchpanel_priv {
	int size_x;
	int size_y;
	int max_x;
	int max_y;
	bool inverted_x;
	bool inverted_y;
	bool swapped_x_y;
};

struct touchpanel_touch {
	int id;
	int x;
	int y;
};

/**
 * struct touchpanel_ops - touchpanel device operations
 */
struct touchpanel_ops {
	/**
	 * start() - enable the touchpanel to be ready for use
	 *
	 * @dev:	Device to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*start)(struct udevice *dev);

	/**
	 * stop() - disable the touchpanel when no-longer needed
	 *
	 * @dev:	Device to disable
	 * @return 0 if OK, -ve on error
	 */
	int (*stop)(struct udevice *dev);

	/**
	 * get_touches() - get list of active touches
	 *
	 * @dev:	Device to read from
	 * @touches:	Array where to store touches. If NULL, the driver will
	 *              only return number of touches available.
	 * @max_touches:	Size of an touches array
	 * @return -EAGAIN if no touch is available, otherwise number of touches
	 * available.
	 */
	int (*get_touches)(struct udevice *dev,
			   struct touchpanel_touch* touches, int max_touches);
};

#define touchpanel_get_ops(dev)	((struct touchpanel_ops *)(dev)->driver->ops)

int touchpanel_start(struct udevice *dev);
int touchpanel_stop(struct udevice *dev);
int touchpanel_get_touches(struct udevice *dev,
			   struct touchpanel_touch* touches, int max_touches);

#endif /* __TOUCHPANEL_H */
