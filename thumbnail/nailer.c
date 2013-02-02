#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bsd/md5.h>

#include <Imlib2.h>

#include <giblib/giblib.h>

#include "thumbnail/feh_png.h"

#include "common/imlib_error.h"
#include "thumbnail/thumbnail.h"

static Imlib_Image regenerate_from_original(const char *filename, int size) {
	/* load image */

	Imlib_Load_Error err;
	Imlib_Image orig = imlib_load_image_with_error_return(filename, &err);

	if (orig == NULL) {
		const char *errmsg = imlib_load_error_string(err);
		warnx("couldn't load %s for thumbnailing: %s", filename, errmsg);
		return NULL; /* XXX do what with error message? extra param? */
	}

	imlib_context_set_image(orig);

	struct coord image_dim = COORD(
		imlib_image_get_width(),
		imlib_image_get_height()
	);

	/* make thumbnail, but without upscaling smaller images. */

	struct coord thumb_dim = coord_downscale_to_fit(image_dim, COORD(size, size));

	Imlib_Image thumb = imlib_create_cropped_scaled_image(0, 0,
		image_dim.width, image_dim.height,
		thumb_dim.width, thumb_dim.height
	);

	/* can free original image now */

	imlib_context_set_image(orig);
	imlib_free_image();

	/* done */

	if (thumb == NULL) {
		warnx("error while downscaling %s for thumbnailing", filename);
		return NULL;
	}

	return thumb;
}

static char *tms_uri_for_filename(const char *filename) {
	char *uri;

	char *rp = realpath(filename, NULL);
	asprintf(&uri, "file://%s", rp);
	free(rp);

	return uri;
}

static char *tms_thumbfile_for_filename(const char *filename, int size) {
	char *sizedir;
	if (size == 256) {
		sizedir = "large";
	} else if (size == 128) {
		sizedir = "normal";
	} else {
		warnx("requested thumbnail size %d which is not supported by the thumbnail managing standard", size);
		/* FIXME */
		sizedir = "normal";
	}

	/* FIXME ignore a "filename" in the thumbnail directory */

	/* FIXME FIXME use the right thumbnail directory */

	char *thumbdir = "/home/penma/.thumbnails";

	char *uri = tms_uri_for_filename(filename);

	/* hash the URI */

	char *digest = MD5Data(uri, strlen(uri), NULL);

	/* now we can build the filename */
	char *fn;
	asprintf(&fn, "%s/%s/%s.png", thumbdir, sizedir, digest);

	free(uri);
	free(digest);

	return fn;
}

static Imlib_Image check_cached(const char *filename, int size) {
	/* stat the original file. necessary to verify if thumbnail
	 * is up to date */
	struct stat st;
	if (stat(filename, &st)) {
		warn("cannot stat original image %s", filename);
		return NULL;
	}

	/* where do we expect the thumbnail file? */

	char *thumbfile = tms_thumbfile_for_filename(filename, size);

	/* poke it and load it. FIXME */

	gib_hash *attr = feh_png_read_comments(thumbfile);

	if (attr == NULL) {
		/* the thumbnail file probably doesn't exist */
		return NULL;
	}

	char *mtimec = (char *) gib_hash_get(attr, "Thumb::MTime");
	time_t mtime = atol(mtimec);
	gib_hash_free_and_data(attr);

	if (mtime != st.st_mtime) {
		/* wrong mtime. or invalid one. either way, not useful */
		return NULL;
	}

	/* looks legit, load it */
	Imlib_Load_Error err;
	Imlib_Image thumb = imlib_load_image_with_error_return(thumbfile, &err);

	if (thumb == NULL) {
		const char *errmsg = imlib_load_error_string(err);
		warnx("error loading thumbnail %s: %s", thumbfile, errmsg);
		return NULL;
	}

	return thumb;
}

static void write_cache(const char *filename, int size, Imlib_Image thumb) {
	/* needs original mtime */
	struct stat st;
	if (stat(filename, &st)) {
		warn("cannot stat original image %s", filename);
		return;
	}

	/* write where? */

	char *thumbfile = tms_thumbfile_for_filename(filename, size);

	/* stuff for the png comments */

	char *uri = tms_uri_for_filename(filename);
	char *mtime;
	asprintf(&mtime, "%ld", st.st_mtime);
	feh_png_write_png(thumb, thumbfile,
		"Thumb::URI", uri,
		"Thumb::MTime", mtime,
	NULL);
}

Imlib_Image thumbnail_from_storage(const char *filename, int size) {
	Imlib_Image thumb;

	/* check if a suitable cached thumbnail exists */

	thumb = check_cached(filename, size);

	if (thumb != NULL) {
		return thumb;
	}

	/* nope. we must generate it. */

	thumb = regenerate_from_original(filename, size);

	if (thumb == NULL) {
		return NULL;
	}

	/* we will also cache it */

	write_cache(filename, size, thumb);

	return thumb;
}

