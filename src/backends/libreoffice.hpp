#include "backend.hpp"

struct LibreofficeSettings {

};

const std::vector<enum FileFormat> libreoffice_all = {
    EPUB, HTML, MD, ODT, ODF, PDF, DOCX, DOC, TXT,
};

const std::map<enum FileFormat, std::vector<enum FileFormat>> LIBREOFFICE_CONVERSIONS = {
    {EPUB, libreoffice_all}, {HTML, libreoffice_all}, {MD, libreoffice_all}, {ODT, libreoffice_all},
    {ODF, libreoffice_all}, {PDF, {HTML}}, {DOCX, libreoffice_all}, {DOC, libreoffice_all}, {TXT, libreoffice_all},
};

enum Result libreoffice_convert_single(std::string in_path, std::string output_folder, std::string extension, struct LibreofficeSettings settings);
void libreoffice_set_settings_widget(GtkWidget* box);
struct LibreofficeSettings libreoffice_get_settings(GtkWidget* box);
