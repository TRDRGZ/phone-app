## 実行方法
### 1. リポジトリをクローンする
```bash
git clone https://github.com/26ryss/phone-app
```
### 2. 仮想環境を構築する
一度仮想環境のファイルを削除する
```bash
rm -rf venv
```

仮想環境を作成
```bash
python -m venv venv
```
仮想環境に入る
```bash
source venv/bin/activate
```

ライブラリをインポートする
```bash
pip install flask
```

### 3. 実行
コードを実行する
```bash
python run.py
```

以下のような出力がでるので表示されたURLにブラウザでアクセスする
```
 * Serving Flask app 'app.app'
 * Debug mode: on
WARNING: This is a development server. Do not use it in a production deployment. Use a production WSGI server instead.
 * Running on http://127.0.0.1:5000
Press CTRL+C to quit
 * Restarting with stat
 * Debugger is active!
 * Debugger PIN: 199-875-315
```