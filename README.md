# Insert Memo

DAW のインサートに挿すメモ帳 VST3 エフェクトです。音声は一切加工せずそのまま通過させます（レイテンシ 0）。

- 書いた内容は DAW のプロジェクトに保存されます
- プレーンテキスト専用です。マークアップ・スクリプト・マクロなど、中で何かを実行する仕組みは一切ありません
- `http://` `https://` `www.` で始まる URL はリンクになります（開くのは http / https のみ）

## 使い方

| 操作 | 内容 |
|---|---|
| ［編集］／［表示］ボタン | モード切替 |
| 表示モードで URL をクリック | ブラウザで開く |
| 表示モードで本文をダブルクリック | 編集モードへ |
| 編集モードで Ctrl+クリック | カーソル位置の URL を開く |
| A- / A+ | 文字サイズ変更 |
| ウィンドウ右下 | サイズ変更（サイズも保存されます） |

## インストール（テスト用）

Actions の成果物 zip を展開し、`install_for_test.bat` を実行してから DAW でプラグインを再スキャンしてください。
インストール先: `%LOCALAPPDATA%\Programs\Common\VST3`

## ビルド

```
cmake -B build -A x64
cmake --build build --config Release
```

JUCE 8.0.6 は CMake が自動でダウンロードします（`./JUCE` があればそちらを使用）。

## ライセンス

This software is licensed under the AGPL-3.0.
It uses the JUCE framework (AGPLv3 / commercial dual licence) and the Steinberg VST 3 SDK.
VST is a trademark of Steinberg Media Technologies GmbH.
