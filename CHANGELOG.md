<a name="1.6.0"></a>
# 1.6.0 (2026-01-06)


### Bug Fixes

* **cmake:** call cmake_minimum_required() piror to project() ([#198](https://github.com/rime/ibus-rime/issues/198)) ([4edca6d](https://github.com/rime/ibus-rime/commit/4edca6d))
* **rime_main:** Adjust the sigterm_cb processing logic ([#205](https://github.com/rime/ibus-rime/issues/205)) ([4218655](https://github.com/rime/ibus-rime/commit/4218655))


### Features

* **rime_engine:** change candidate selection horizontally ([dc5a9a1](https://github.com/rime/ibus-rime/commit/dc5a9a1))



<a name="1.5.1"></a>
### 1.5.1 (2024-12-29)


#### Bug Fixes

* **rime_engine:**  ignore super modifier in gtk4 (#192)



<a name="1.5.0"></a>
## 1.5.0 (2021-01-24)


#### Features

* **ibus_rime.yaml:**  install config file for ibus-rime, set default UI style
* **plugins:**  use librime's internal plugin loader
* **rime_settings:**  add config options: style/preedit_style, style/cursor_type (closes #102)
* **submodules:**  migrate to rime/plum

#### Breaking Changes

* **plugins:**  use librime's internal plugin loader

#### Bug Fixes

* **CMakeLists.txt:**  static build requires boost_locale and icuuc
* **logging:**  wrong log file name
* **rime_engine:**  ignore keys with super modifier
* **rime_main.cc:**  deploy ibus_rime.yaml on maintenance
* **rime_settings.h:**  compile with gcc 10 (closes #90)



<a name="1.4.0"></a>
## 1.4.0 (2019-01-28)


* migrate to librime 1.x API
* **rime.xml:**  update ibus component name to `im.rime.Rime`

#### Bug Fixes

* **rime_main.cc:**  deploy ibus_rime.yaml on maintenance ([8e081264](https://github.com/rime/ibus-rime/commit/8e081264977bfefdd233ebeb1235f11be55b28f8))



<a name="1.3.0"></a>
## 1.3.0 (2018-04-22)


#### Features

* **ui:**
  *  select candidate with mouse/touch screen
  *  support page up/down buttons in candidate panel
  *  toggle ascii mode by clicking button
* **rime_settings:** load settings from `ibus_rime.yaml`; deprecate `IBusConfig`
* **submodules:**  migrate to rime/plum ([5808a2b1](https://github.com/rime/ibus-rime/commit/5808a2b1d0ccac99607ad9985de0675963387bda))



## 1.2 (2013-07-19)

* **rime_engine.c**: add status bar icons; update status bar only when there is
  a change in IME status, to fix issues on Ubuntu Trusty.
* **rime_config.h**: define macros for version and file paths.
* **Makefile**: fix file permissions in install target.



## 1.1 (2013-12-26)

* **rime.xml**: add symbolic icon for GNOME Shell.
* **rime_settings**: fallback to ibus default UI options in `panel` section.



## 1.0 (2013-11-10)

* **librime**: update to version 1.0
* **rime_settings**: add config option `lookup_table_orientation`.



## 0.9.9 (2013-05-05)

* **brise**: update to version 0.22
* **librime**: update to version 0.9.9
* **Makefile**: support static linking librime and its dependencies.



## 0.9.8 (2013-02-02)

* Release.



## 0.9.7 (2013-01-16)

* **rime_engine.c**: support changing conversion mode with Caps Lock.
* **librime**: updated to version 0.9.7.



## 0.9.6 (2013-01-12)

* **rime_engine.c**: add 'Sync data' button.
  in Chinese mode, display the name of schema on status bar.
* **rime_main.c**: display notifications about deployment result.



## 0.9.4 (2012-09-26)

* new dependency: brise.



## 0.9.2 (2012-07-08)

* **UI**: new Rime logo.
* **UI**: add Chinese/ascii mode indicator, and Deploy button to status bar.
* **UI**: show notification when deploying Rime.
