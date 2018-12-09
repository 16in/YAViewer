@rem YAViewerを完全にインストール時の設定に戻します。
@rem YAViewerが生成した全ての中間ファイルと設定をクリアします。

@call delete_setting.bat
@rmdir /s /q temporary

@exit /b
