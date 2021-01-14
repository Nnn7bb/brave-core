/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/cosmetic_filters/content/renderer/cosmetic_filters_js_handler.h"

#include <string>

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_frame.h"
#include "gin/arguments.h"
#include "gin/function_template.h"
#include "third_party/blink/public/common/browser_interface_broker_proxy.h"
#include "third_party/blink/public/web/blink.h"
#include "v8/include/v8.h"

namespace cosmetic_filters {

CosmeticFiltersJSHandler::CosmeticFiltersJSHandler(
    content::RenderFrame* render_frame)
    : render_frame_(render_frame) {}

CosmeticFiltersJSHandler::~CosmeticFiltersJSHandler() = default;

void CosmeticFiltersJSHandler::HiddenClassIdSelectors(
    const std::string& input) {
  if (cosmetic_filters_resources_) {
    cosmetic_filters_resources_->HiddenClassIdSelectors(
        input,
        base::BindOnce(&CosmeticFiltersJSHandler::OnHiddenClassIdSelectors,
                       base::Unretained(this)));
  }
}

void CosmeticFiltersJSHandler::OnHiddenClassIdSelectors(base::Value result) {}

void CosmeticFiltersJSHandler::AddJavaScriptObjectToFrame(
    v8::Local<v8::Context> context) {
  v8::Isolate* isolate = blink::MainThreadIsolate();
  v8::HandleScope handle_scope(isolate);
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);

  v8::Local<v8::Object> worker_obj = GetOrCreateWorkerObject(isolate, context);

  EnsureConnected();

  BindFunctionToObject(
      isolate, worker_obj, "hiddenClassIdSelectors",
      base::BindRepeating(&CosmeticFiltersJSHandler::HiddenClassIdSelectors,
                          base::Unretained(this)));
}

template <typename Sig>
void CosmeticFiltersJSHandler::BindFunctionToObject(
    v8::Isolate* isolate,
    v8::Local<v8::Object> javascript_object,
    const std::string& name,
    const base::RepeatingCallback<Sig>& callback) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  // Get the isolate associated with this object.
  javascript_object
      ->Set(context, gin::StringToSymbol(isolate, name),
            gin::CreateFunctionTemplate(isolate, callback)
                ->GetFunction(context)
                .ToLocalChecked())
      .Check();
}

void CosmeticFiltersJSHandler::EnsureConnected() {
  if (!cosmetic_filters_resources_) {
    render_frame_->GetBrowserInterfaceBroker()->GetInterface(
        cosmetic_filters_resources_.BindNewPipeAndPassReceiver());
  }
}

v8::Local<v8::Object> GetOrCreateWorkerObject(v8::Isolate* isolate,
                                              v8::Local<v8::Context> context) {
  v8::Local<v8::Object> global = context->Global();
  v8::Local<v8::Object> cosmetic_filters_obj;
  v8::Local<v8::Value> cosmetic_filters_value;
  if (!global->Get(context, gin::StringToV8(isolate, "cf_worker"))
           .ToLocal(&cosmetic_filters_value) ||
      !cosmetic_filters_value->IsObject()) {
    cosmetic_filters_obj = v8::Object::New(isolate);
    global
        ->Set(context, gin::StringToSymbol(isolate, "cf_worker"),
              cosmetic_filters_obj)
        .Check();
  } else {
    cosmetic_filters_obj = v8::Local<v8::Object>::Cast(cosmetic_filters_value);
  }

  return cosmetic_filters_obj;
}

}  // namespace cosmetic_filters
