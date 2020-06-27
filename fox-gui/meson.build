fox_dep = dependency('fox')
agg_dep = dependency('libagg')
libelemfox_dep = dependency('libelemfox')

foxgui_incdir = include_directories('.')

fox_src = ['disp-lookup-components.cpp', 'dispers_sampling_optim.cpp', 'ProgressInfo.cpp', 'spectrum_plot.cpp', 'disp_fit_window.cpp', 'regress_pro_window.cpp', 'dispers_win.cpp', 'sampling_win.cpp', 'fx_numeric_field.cpp', 'fit_panel.cpp', 'fit_window.cpp', 'colors.cpp', 'plot_units.cpp', 'plot_canvas.cpp', 'dispers_save_dialog.cpp', 'filmstack_window.cpp', 'dispers_ui_edit.cpp', 'icons_all.cpp', 'dispers_chooser.cpp', 'fit_params_utils.cpp', 'multi_sample_recipe.cpp', 'recipe_window.cpp', 'fit_recipe.cpp', 'dispers_edit_window.cpp', 'dispers_ui_utils.cpp', 'filelist_table.cpp', 'dataset_table.cpp', 'dataset_window.cpp', 'batch_window.cpp', 'regress_pro.cpp', 'testing_app.cpp', 'regress_pro_testing.cpp', 'main.cpp']

executable('regress-pro', fox_src,
           include_directories: [foxgui_incdir, kernel_incdir],
           dependencies: [gsl_dep, nlopt_dep, fox_dep, agg_dep, libelemfox_dep, thread_dep],
           link_with: libkernel,
           link_args: regpro_link_args,
           gui_app: not debug_build, install: true)
