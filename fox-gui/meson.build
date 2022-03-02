regpro_gui_deps = []

regpro_gui_deps += dependency('fox')
libagg_dep = dependency('libagg', fallback: ['libagg', 'libagg_dep'])
regpro_gui_deps += libagg_dep

regpro_cpp_args = regpro_core_args

fox_src = [
    'ProgressInfo.cpp',
    'spectrum_plot.cpp',
    'disp_fit_window.cpp',
    'regress_pro_window.cpp',
    'dispers_win.cpp',
    'sampling_win.cpp',
    'fx_numeric_field.cpp',
    'fit_panel.cpp',
    'fit_window.cpp',
    'colors.cpp',
    'plot_units.cpp',
    'plot_canvas.cpp',
    'utils.cpp',
    'dispers_save_dialog.cpp',
    'filmstack_window.cpp',
    'dispers_ui_edit.cpp',
    'icons_all.cpp',
    'dispers_chooser.cpp',
    'fit_params_utils.cpp',
    'multi_sample_recipe.cpp',
    'recipe_window.cpp',
    'fit_recipe.cpp',
    'dispers_edit_window.cpp',
    'dispers_ui_utils.cpp',
    'filelist_table.cpp',
    'dataset_table.cpp',
    'dataset_window.cpp',
    'batch_window.cpp',
    'regress_pro.cpp',
    'main.cpp'
]

executable('regress-pro', fox_src,
    include_directories: libell_include,
    dependencies: regpro_core_deps + regpro_gui_deps,
    link_with: libell,
    link_args: regpro_link_args,
    cpp_args: regpro_cpp_args,
    gui_app: not debug_build,
    install: true
)
