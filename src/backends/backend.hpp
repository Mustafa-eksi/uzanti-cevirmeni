#ifndef BACKEND_H
#define BACKEND_H
#include "gtk/gtkdropdown.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <gtk/gtk.h>
#define SMOL_BUFF 256
#define BIG_BUFF 1024

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

enum Result {
    SUCCESS,
    UNKNOWN_ERROR,
    EXECUTABLE_NOT_FOUND,
    NOT_ENOUGH_MEMORY,
    FILE_NOT_FOUND,
};

enum FileFormat {
    WEBM, GIF, MP4, MP3, OPUS, AVI, MOV,
    PNG, WEBP, JPG, JPEG, SVG, EPUB, HTML,
    JSON, TEX, MD, ODT, OPML, ORG, RST,
    ODF, PDF, DOCX, DOC, TXT, XCF,
};
const std::map<enum FileFormat, std::string> FORMAT_EXTENSIONS = {
    {WEBM, "webm"}, {GIF, "gif"}, {MP4, "mp4"}, {MP3, "mp3"}, {OPUS, "opus"},
    {AVI, "avi"}, {MOV, "mov"}, {PNG, "png"}, {WEBP, "webp"}, {JPG, "jpg"},
    {JPEG, "jpeg"}, {SVG, "svg"}, {EPUB, "epub"}, {HTML, "html"}, {JSON, "json"},
    {TEX, "tex"}, {MD, "md"}, {ODT, "odt"}, {OPML, "opml"}, {ORG, "org"},
    {RST, "rst"}, {ODF, "odf"}, {PDF, "pdf"}, {DOCX, "docx"}, {DOC, "doc"},
    {TXT, "txt"}, {XCF, "xcf"}
};
const std::map<enum FileFormat, std::string> FORMAT_MIME_TYPES = {
    {WEBM, "video/webm"}, {GIF, "image/gif"}, {MP4, "video/mp4"}, {MP3, "audio/mp3"}, {OPUS, "audio/opus"},
    {AVI, "video/x-msvideo"}, {MOV, "video/quicktime"}, {PNG, "image/png"}, {WEBP, "image/webp"}, {JPG, "image/jpeg"},
    {JPEG, "image/jpeg"}, {SVG, "image/svg+xml"}, {EPUB, "application/epub+zip"}, {HTML, "text/html"}, {JSON, "application/json"},
    {TEX, "application/x-tex"}, {MD, "text/markdown"}, {ODT, "application/vnd.oasis.opendocument.text"}, {OPML, "text/xml"}, {ORG, "text/org"},
    {RST, "text/rst"}, {PDF, "application/pdf"}, {DOCX, "application/vnd.openxmlformats-officedocument.wordprocessingml.document"}, {DOC, "application/msword"},
    {TXT, "text/plain"}, {XCF, "image/x-xcf"}
};

char *change_extension(const char* file, const char* new_extension, char* out)
{
    size_t dot_pos = 0;
    for (size_t i = 0; i < strlen(file); i++)
        if (file[i] == '.')
            dot_pos = i;
    // char *out = (char*) malloc((dot_pos+strlen(new_extension))*sizeof(char));
    strcpy(out, file+dot_pos+1);
    strcpy(out+dot_pos+1, new_extension);
    return out;
}

GtkWidget *label_spin(std::string label, int rs, int re, int s, int starting_value) {
    GtkWidget *rotbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *rotlabw = gtk_label_new(label.c_str());
    GtkWidget *rotspinw = gtk_spin_button_new_with_range(rs, re, s);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rotspinw), starting_value);
    gtk_box_append(GTK_BOX(rotbox), rotlabw);
    gtk_box_append(GTK_BOX(rotbox), rotspinw);
    return rotbox;
}

int get_spin_value(GtkWidget *box) {
    auto spinw = gtk_widget_get_last_child(box);
    return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinw));
}

GtkWidget *label_drop(std::string label, const char** strings) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *labw = gtk_label_new(label.c_str());
    GtkWidget *dropw = gtk_drop_down_new_from_strings(strings);
    gtk_box_append(GTK_BOX(box), labw);
    gtk_box_append(GTK_BOX(box), dropw);
    return box;
}

size_t get_drop_value(GtkWidget *box) {
    auto dropw = gtk_widget_get_last_child(box);
    return gtk_drop_down_get_selected(GTK_DROP_DOWN(dropw));
}

void set_equal_margin(GtkWidget* w, size_t m) {
    gtk_widget_set_margin_bottom(w, m);
    gtk_widget_set_margin_top(w, m);
    gtk_widget_set_margin_start(w, m);
    gtk_widget_set_margin_end(w, m);
}

#endif // BACKEND_H
