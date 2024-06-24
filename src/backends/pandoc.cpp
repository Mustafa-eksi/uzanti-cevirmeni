#include "pandoc.hpp"

const char *pandoc_cmd_template = "pandoc \"%s\" -o \"%s\"";

std::string pandoc_parse_settings(struct PandocSettings settings) {
    std::string output;
    if (settings.hard_line_breaks) {
        output += " --wrap=preserve ";
    }
    return output;
}

enum Result pandoc_convert_single(const char* in_path, const char* out_path, struct PandocSettings settings) {
    enum Result res = UNKNOWN_ERROR;
    std::string cmd =  "pandoc \""+(std::string)in_path+"\" -o \""+(std::string)out_path+"\" " + pandoc_parse_settings(settings);

    FILE *pandoc_pipe = popen(cmd.c_str(), "r");
    if (!pandoc_pipe) {
        return UNKNOWN_ERROR;
    }

    char output[BIG_BUFF] = {};

    // TODO: Read pandoc output for logging and error detection
    fgets(output, sizeof(output), pandoc_pipe);

    // FIXME: there can be cases of pandoc returning 0 even if an error occurred.
    if (pclose(pandoc_pipe) == 0)
        return SUCCESS;
    printf("Pandoc error: %s\n", output);
    return res;
}

void pandoc_set_settings_widget(GtkWidget* grid) {
    gtk_grid_insert_column(GTK_GRID(grid), 0);
    gtk_grid_insert_row(GTK_GRID(grid), 0);
    GtkWidget* wrap = gtk_check_button_new();
    gtk_check_button_set_active(GTK_CHECK_BUTTON(wrap), true);
    gtk_check_button_set_label(GTK_CHECK_BUTTON(wrap), _("Preserve wraps")); // Yeni satırları koru
    gtk_grid_attach(GTK_GRID(grid), wrap, 0, 0, 1, 1);
}

struct PandocSettings pandoc_get_settings(GtkWidget* box) {
    GtkWidget* wrap = gtk_widget_get_first_child(box);
    return {
        .hard_line_breaks = (bool) gtk_check_button_get_active(GTK_CHECK_BUTTON(wrap)),
    };
}
