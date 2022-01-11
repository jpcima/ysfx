#include "ysfx.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;

    ysfx_menu_u menu{ysfx_parse_menu(argv[1])};
    if (!menu)
        return 1;

    int menulevel = 0;

    auto indent = [&menulevel]() {
        for (int i = 0, n = menulevel; i < n; ++i)
            printf("\t");
    };

    auto yesno = [](bool b) {
        return b ? "yes" : "no";
    };

    for (uint32_t i = 0; i < menu->insn_count; ++i) {
        ysfx_menu_insn_t &insn = menu->insns[i];

        switch (insn.opcode) {
        case ysfx_menu_item:
            indent(); printf("+ Item\n");
            indent(); printf("| ID: %d\n", insn.id);
            indent(); printf("| Name: %s\n", insn.name);
            indent(); printf("| Disabled: %s\n", yesno(insn.item_flags & ysfx_menu_item_disabled));
            indent(); printf("| Checked: %s\n", yesno(insn.item_flags & ysfx_menu_item_checked));
            break;
        case ysfx_menu_separator:
            indent(); printf("+ Separator\n");
            break;
        case ysfx_menu_sub:
            indent(); printf("+ Submenu start\n");
            indent(); printf("| Name: %s\n", insn.name);
            indent(); printf("| Disabled: %s\n", yesno(insn.item_flags & ysfx_menu_item_disabled));
            indent(); printf("| Checked: %s\n", yesno(insn.item_flags & ysfx_menu_item_checked));
            ++menulevel;
            break;
        case ysfx_menu_endsub:
            --menulevel;
            indent(); printf("+ Submenu end\n");
            indent(); printf("| Name: %s\n", insn.name);
            indent(); printf("| Disabled: %s\n", yesno(insn.item_flags & ysfx_menu_item_disabled));
            indent(); printf("| Checked: %s\n", yesno(insn.item_flags & ysfx_menu_item_checked));
            break;
        }
    }

    return 0;
}
