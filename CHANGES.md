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


