// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_HTTP_BasicBPLibrary.h"
#include "FF_HTTP_Basic.h"

// UE Includes.
#include "Misc/App.h"
#include "Misc/FileHelper.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"     // Join string arrays for headers.

// UE HTTP Client Basic.
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

THIRD_PARTY_INCLUDES_START
#include "curl/curl.h"
#include <string>                           // Message Id
#include <iostream>                         // Message Id
#include <random>                           // Message Id
#include <algorithm>                        // Message Id
THIRD_PARTY_INCLUDES_END

UFF_HTTP_BasicBPLibrary::UFF_HTTP_BasicBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

FString UFF_HTTP_BasicBPLibrary::HTTP_Response_Lenght(FString In_Response)
{
    if (In_Response.IsEmpty())
    {
        return "0";
    }

    return FString::FromInt(FTCHARToUTF8(In_Response.GetCharArray().GetData()).Length());
}

FString UFF_HTTP_BasicBPLibrary::HTTP_Content_Types_To_String(EHttpContentTypes In_Types)
{
    switch (In_Types)
    {
    case EHttpContentTypes::None:
        return "";
    case EHttpContentTypes::PDF:
        return "application/pdf";
    case EHttpContentTypes::JSON:
        return "application/json";
    case EHttpContentTypes::TEXT:
        return "text/plain";
    case EHttpContentTypes::HTML:
        return "text/html";
    default:
        return "";
    }
}

int32 UFF_HTTP_BasicBPLibrary::HTTP_Convert_Response_Codes(EHttpResponseCodesBp ResponseCodes)
{
    switch (ResponseCodes)
    {
    case EHttpResponseCodesBp::Unknown:
        return 0;
    case EHttpResponseCodesBp::Continue:
        return 100;
    case EHttpResponseCodesBp::SwitchProtocol:
        return 101;
    case EHttpResponseCodesBp::Ok:
        return 200;
    case EHttpResponseCodesBp::Created:
        return 201;
    case EHttpResponseCodesBp::Accepted:
        return 202;
    case EHttpResponseCodesBp::Partial:
        return 203;
    case EHttpResponseCodesBp::NoContent:
        return 204;
    case EHttpResponseCodesBp::ResetContent:
        return 205;
    case EHttpResponseCodesBp::PartialContent:
        return 206;
    case EHttpResponseCodesBp::Ambiguous:
        return 300;
    case EHttpResponseCodesBp::Moved:
        return 301;
    case EHttpResponseCodesBp::Redirect:
        return 302;
    case EHttpResponseCodesBp::RedirectMethod:
        return 303;
    case EHttpResponseCodesBp::NotModified:
        return 304;
    case EHttpResponseCodesBp::UseProxy:
        return 305;
    case EHttpResponseCodesBp::RedirectKeepVerb:
        return 307;
    case EHttpResponseCodesBp::BadRequest:
        return 400;
    case EHttpResponseCodesBp::Denied:
        return 401;
    case EHttpResponseCodesBp::PaymentReq:
        return 402;
    case EHttpResponseCodesBp::Forbidden:
        return 403;
    case EHttpResponseCodesBp::NotFound:
        return 404;
    case EHttpResponseCodesBp::BadMethod:
        return 405;
    case EHttpResponseCodesBp::NoneAcceptable:
        return 406;
    case EHttpResponseCodesBp::ProxyAuthReq:
        return 407;
    case EHttpResponseCodesBp::RequestTimeout:
        return 408;
    case EHttpResponseCodesBp::Conflict:
        return 409;
    case EHttpResponseCodesBp::Gone:
        return 410;
    case EHttpResponseCodesBp::LengthRequired:
        return 411;
    case EHttpResponseCodesBp::PrecondFailed:
        return 412;
    case EHttpResponseCodesBp::RequestTooLarge:
        return 413;
    case EHttpResponseCodesBp::UriTooLong:
        return 414;
    case EHttpResponseCodesBp::UnsupportedMedia:
        return 415;
    case EHttpResponseCodesBp::TooManyRequests:
        return 429;
    case EHttpResponseCodesBp::RetryWith:
        return 449;
    case EHttpResponseCodesBp::ServerError:
        return 500;
    case EHttpResponseCodesBp::NotSupported:
        return 501;
    case EHttpResponseCodesBp::BadGateway:
        return 502;
    case EHttpResponseCodesBp::ServiceUnavail:
        return 503;
    case EHttpResponseCodesBp::GatewayTimeout:
        return 504;
    case EHttpResponseCodesBp::VersionNotSup:
        return 505;
    default:
        return 0;
    }
}

void UFF_HTTP_BasicBPLibrary::FF_HTTP_Client_Basic_Bytes(FDelegateHttpClient DelegateClient, FString In_Url, TMap<FString, FString> In_Header, TArray<uint8> In_Body, EHttpRequestTypes In_Request_Type, EHttpContentTypes ContentType, bool bAddDefaultHeaders)
{
    if (In_Url.IsEmpty())
    {
        return;
    }

    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateClient, In_Request_Type, In_Url, In_Header, In_Body, ContentType, bAddDefaultHeaders]()
        {
            FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();

            switch (In_Request_Type)
            {
            case EHttpRequestTypes::None:
                break;
            case EHttpRequestTypes::GET:
                HttpRequest->SetVerb("GET");
                break;
            case EHttpRequestTypes::POST:
                HttpRequest->SetVerb("POST");
                break;
            case EHttpRequestTypes::PUT:
                HttpRequest->SetVerb("PUT");
                break;
            case EHttpRequestTypes::DEL:
                HttpRequest->SetVerb("Delete");
                break;
            default:
                HttpRequest->SetVerb("GET");
                break;
            }

            FString Content_Type_String = UFF_HTTP_BasicBPLibrary::HTTP_Content_Types_To_String(ContentType);
            if (!Content_Type_String.IsEmpty())
            {
                HttpRequest->AppendToHeader("Contenty-Type", Content_Type_String);
            }
            
            HttpRequest->SetURL(In_Url);
            HttpRequest->SetContent(In_Body);

            for (TPair<FString, FString> Pair_Headers : In_Header)
            {
                HttpRequest->AppendToHeader(Pair_Headers.Key, Pair_Headers.Value);
            }

            if (bAddDefaultHeaders)
            {
                HttpRequest->AppendToHeader("Cache-Control", "no-cache");
                HttpRequest->AppendToHeader("Accept", "*/*");
                HttpRequest->AppendToHeader("Accept-Encoding", "gzip, deflate, br");
                HttpRequest->AppendToHeader("Connection", "keep-alive");
            }
            
            HttpRequest->OnProcessRequestComplete().BindLambda([DelegateClient](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
                {
                    AsyncTask(ENamedThreads::GameThread, [DelegateClient, Response, bWasSuccessful]()
                        {
                            FHttpClientResponse ResponseStruct;
                            if (bWasSuccessful)
                            {
                                ResponseStruct.Headers = Response->GetAllHeaders();
                                ResponseStruct.Content = Response->GetContent();
                                ResponseStruct.ContentString = Response->GetContentAsString();
                                ResponseStruct.ContentLenght = Response->GetContentLength();
                                ResponseStruct.ContentType = Response->GetContentType();
                                //ResponseStruct.Url = Response->GetURL().IsEmpty() ? "" : Response->GetURL(); This gives error.
                                ResponseStruct.ResponseCode = Response->GetResponseCode();
                            }

                            DelegateClient.ExecuteIfBound(bWasSuccessful, ResponseStruct);
                        }
                    );
                }
            );

            HttpRequest->ProcessRequest();
        }
    );
}

void UFF_HTTP_BasicBPLibrary::FF_HTTP_Client_Basic_String(FDelegateHttpClient DelegateClient, FString In_Url, TMap<FString, FString> In_Header, FString In_Body, EHttpRequestTypes In_Request_Type, EHttpContentTypes ContentType, bool bAddDefaultHeaders)
{
    if (In_Url.IsEmpty())
    {
        return;
    }

    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateClient, In_Request_Type, In_Url, In_Header, In_Body, ContentType, bAddDefaultHeaders]()
        {
            FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();

            switch (In_Request_Type)
            {
            case EHttpRequestTypes::None:
                break;
            case EHttpRequestTypes::GET:
                HttpRequest->SetVerb("GET");
                break;
            case EHttpRequestTypes::POST:
                HttpRequest->SetVerb("POST");
                break;
            case EHttpRequestTypes::PUT:
                HttpRequest->SetVerb("PUT");
                break;
            case EHttpRequestTypes::DEL:
                HttpRequest->SetVerb("Delete");
                break;
            default:
                HttpRequest->SetVerb("GET");
                break;
            }

            FString Content_Type_String = UFF_HTTP_BasicBPLibrary::HTTP_Content_Types_To_String(ContentType);
            if (!Content_Type_String.IsEmpty())
            {
                HttpRequest->AppendToHeader("Contenty-Type", Content_Type_String);
            }

            HttpRequest->SetURL(In_Url);
            HttpRequest->SetContentAsString(In_Body);

            for (TPair<FString, FString> Pair_Headers : In_Header)
            {
                HttpRequest->AppendToHeader(Pair_Headers.Key, Pair_Headers.Value);
            }

            if (bAddDefaultHeaders)
            {
                HttpRequest->AppendToHeader("Cache-Control", "no-cache");
                HttpRequest->AppendToHeader("Accept", "*/*");
                HttpRequest->AppendToHeader("Accept-Encoding", "gzip, deflate, br");
                HttpRequest->AppendToHeader("Connection", "keep-alive");
            }

            HttpRequest->OnProcessRequestComplete().BindLambda([DelegateClient](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
                {
                    AsyncTask(ENamedThreads::GameThread, [DelegateClient, Response, bWasSuccessful]()
                        {
                            FHttpClientResponse ResponseStruct;
                            if (bWasSuccessful)
                            {
                                ResponseStruct.Headers = Response->GetAllHeaders();
                                ResponseStruct.Content = Response->GetContent();
                                ResponseStruct.ContentString = Response->GetContentAsString();
                                ResponseStruct.ContentLenght = Response->GetContentLength();
                                ResponseStruct.ContentType = Response->GetContentType();
                                //ResponseStruct.Url = Response->GetURL().IsEmpty() ? "" : Response->GetURL(); this gives error.
                                ResponseStruct.ResponseCode = Response->GetResponseCode();
                            }

                            DelegateClient.ExecuteIfBound(bWasSuccessful, ResponseStruct);
                        }
                    );
                }
            );

            HttpRequest->ProcessRequest();
        }
    );
}

void UFF_HTTP_BasicBPLibrary::LibCurl_Send_Email(FDelegateMailSent DelegateMailSent, TArray<FMailAttachments> Attachments, TArray<FString> To, TArray<FString> Cc, TArray<FString> Bcc, FString Subject, FString Message, FString Sender, FString Password, FString CustomServer, FString In_Cert_Path, EMailServers KnownServers, EMailSecurity CustomServerSecurity)
{
    if (CustomServer.IsEmpty() && In_Cert_Path.IsEmpty())
    {
        DelegateMailSent.ExecuteIfBound(false, "Certificate path is empty.");
        return;
    }

    else if (!CustomServer.IsEmpty() && CustomServerSecurity != EMailSecurity::NoSecurity && In_Cert_Path.IsEmpty())
    {
        DelegateMailSent.ExecuteIfBound(false, "Certificate path is empty.");
        return;
    }

    if (!In_Cert_Path.IsEmpty())
    {
        if (UGameplayStatics::GetPlatformName() == "Windows")
        {
            FPaths::MakeStandardFilename(In_Cert_Path);

            if (!FPaths::FileExists(In_Cert_Path))
            {
                DelegateMailSent.ExecuteIfBound(false, "Certificate couldn't be found.");
                return;
            }

            FPaths::MakePlatformFilename(In_Cert_Path);
        }

        else if (UGameplayStatics::GetPlatformName() == "Android")
        {
            if (!FPaths::FileExists(In_Cert_Path))
            {
                DelegateMailSent.ExecuteIfBound(false, "Certificate couldn't be found.");
                return;
            }
        }
    }

    if (To.Num() == 0)
    {
        DelegateMailSent.ExecuteIfBound(false, "There is no mail recipient to sent.");
        return;
    }

    if (Attachments.Num() > 0)
    {
        int64 Extensions_Size = 0;
        for (int32 Index_Attachments = 0; Index_Attachments < Attachments.Num(); Index_Attachments++)
        {
            Extensions_Size = Extensions_Size + Attachments[Index_Attachments].Base64_Bytes.Num();
        }

        if (Extensions_Size >= (25 * 1024 * 1024))
        {
            DelegateMailSent.ExecuteIfBound(false, ("Attachments are too big.\r\nMax Size = 26214400 Bytes (25 MB).\r\nCurrent Size = " + FString::FromInt(Extensions_Size)));
            return;
        }
    }

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DelegateMailSent, Attachments, To, Cc, Bcc, Subject, Message, Sender, Password, CustomServer, In_Cert_Path, KnownServers, CustomServerSecurity]()
        {
            /*
            *   Initialize CURL Object.
            */

            CURL* Curl = curl_easy_init();
            if (!Curl)
            {
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Curl object couldn't be initialized.");
                    }
                );

                return;
            }

            /*
            *   Server Configurations.
            */

            FString Server;
            if (CustomServer.IsEmpty())
            {
                switch (KnownServers)
                {

                case EMailServers::Gmail:
                    Server = "smtps://smtp.gmail.com:465";
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    break;

                case EMailServers::Outlook:
                    Server = "smtp://smtp-mail.outlook.com:587";
                    curl_easy_setopt(Curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
                    break;

                case EMailServers::Office365:
                    Server = "smtp://smtp.office365.com:587";
                    curl_easy_setopt(Curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
                    break;

                default:
                    Server = "smtps://smtp.gmail.com:465";
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    break;
                }
            }

            else
            {
                Server = CustomServer;

                switch (CustomServerSecurity)
                {

                case EMailSecurity::NoSecurity:
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 0L);
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYHOST, 0L);
                    break;

                case EMailSecurity::TLS:
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    break;

                case EMailSecurity::StartTLS:
                    curl_easy_setopt(Curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
                    break;

                default:
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 0L);
                    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYHOST, 0L);
                    break;
                }
            }

            // Initialize CURL Response.
            CURLcode Response = CURLE_OK;

            Response = curl_easy_setopt(Curl, CURLOPT_URL, TCHAR_TO_UTF8(*Server));
            if (Response != CURLE_OK)
            {
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Server couldn't be defined.");
                    }
                );

                return;
            }

            Response = curl_easy_setopt(Curl, CURLOPT_CAINFO, TCHAR_TO_UTF8(*In_Cert_Path));
            if (Response != CURLE_OK)
            {
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Cacert file couldn't be defined.");
                    }
                );

                return;
            }

            /*
            *   Create Recipients List.
            */

            curl_slist* Recipients = NULL;

            TArray<FString> Array_To;
            for (int32 Index_To = 0; Index_To < To.Num(); Index_To++)
            {
                Array_To.Add("<" + To[Index_To] + ">");
                Recipients = curl_slist_append(Recipients, TCHAR_TO_UTF8(*To[Index_To]));
            }

            TArray<FString> Array_Cc;
            if (Cc.Num() != 0)
            {
                for (int32 Index_Cc = 0; Index_Cc < Cc.Num(); Index_Cc++)
                {
                    Array_Cc.Add("<" + Cc[Index_Cc] + ">");
                    Recipients = curl_slist_append(Recipients, TCHAR_TO_UTF8(*Cc[Index_Cc]));
                }
            }

            TArray<FString> Array_Bcc;
            if (Bcc.Num() != 0)
            {
                for (int32 Index_Bcc = 0; Index_Bcc < Bcc.Num(); Index_Bcc++)
                {
                    Array_Bcc.Add("<" + Bcc[Index_Bcc] + ">");
                    Recipients = curl_slist_append(Recipients, TCHAR_TO_UTF8(*Bcc[Index_Bcc]));
                }
            }

            Response = curl_easy_setopt(Curl, CURLOPT_MAIL_RCPT, Recipients);
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Recipients couldn't be defined.");
                    }
                );

                return;
            }

            Response = curl_easy_setopt(Curl, CURLOPT_MAIL_RCPT_ALLLOWFAILS, 1L);
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Allow recipient fail rule couldn't be defined.");
                    }
                );

                return;
            }

            Response = curl_easy_setopt(Curl, CURLOPT_MAIL_FROM, TCHAR_TO_UTF8(*Sender));
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "From value couldn't be defined.");
                    }
                );

                return;
            }

            Response = curl_easy_setopt(Curl, CURLOPT_USERNAME, TCHAR_TO_UTF8(*Sender));
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Sender value couldn't be defined.");
                    }
                );

                return;
            }

            Response = curl_easy_setopt(Curl, CURLOPT_PASSWORD, TCHAR_TO_UTF8(*Password));
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Password couldn't be defined.");
                    }
                );

                return;
            }

            /*
            *  Header.
            */

            auto Callback_Message_Id = []()->FString
                {
                    const size_t MESSAGE_ID_LEN = 37;

                    tm CurrentTime;
                    time_t TimeClass;
                    time(&TimeClass);

#ifdef _WIN64
                    gmtime_s(&CurrentTime, &TimeClass);

#else
                    gmtime64_r(&TimeClass, &CurrentTime);
#endif

                    std::string ret;
                    ret.resize(MESSAGE_ID_LEN);
                    size_t datelen = std::strftime(&ret[0], MESSAGE_ID_LEN, "%Y%m%d%H%M%S", &CurrentTime);
                    static const std::string alphaNum
                    {
                        "0123456789"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz"
                    };

                    std::mt19937 gen;
                    std::uniform_int_distribution<> dis(0, alphaNum.length() - 1);
                    std::generate_n(ret.begin() + datelen, MESSAGE_ID_LEN - datelen, [&]() { return alphaNum[dis(gen)]; });
                    return ret.c_str();
                };

            curl_slist* Headers = NULL;
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("Subject: " + Subject)));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("Message-ID: <" + Callback_Message_Id() + "@rfcpedant.example.org>")));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("Date: " + FDateTime::Now().ToString())));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("To: " + (Array_To.Num() == 1 ? Array_To[0] : UKismetStringLibrary::JoinStringArray(Array_To, ",")))));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("Cc: " + (Array_Cc.Num() == 1 ? Array_Cc[0] : UKismetStringLibrary::JoinStringArray(Array_Cc, ",")))));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("Bcc: " + (Array_Bcc.Num() == 1 ? Array_Bcc[0] : UKismetStringLibrary::JoinStringArray(Array_Bcc, ",")))));
            Headers = curl_slist_append(Headers, TCHAR_TO_UTF8(*("From: <" + Sender + ">")));
            Response = curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);

            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Headers couldn't be defined.");
                    }
                );

                return;
            }

            /*
            *   Attachments.
            */

            curl_mime* Mime_Mail = curl_mime_init(Curl);
            curl_mimepart* Part;

            if (Attachments.Num() > 0)
            {
                for (int32 Index_Attachment = 0; Index_Attachment < Attachments.Num(); Index_Attachment++)
                {
                    Part = curl_mime_addpart(Mime_Mail);
                    curl_mime_data(Part, reinterpret_cast<const char*>(Attachments[Index_Attachment].Base64_Bytes.GetData()), Attachments[Index_Attachment].Base64_Bytes.Num());
                    curl_mime_encoder(Part, "base64");
                    curl_mime_filename(Part, TCHAR_TO_UTF8(*Attachments[Index_Attachment].Attachment_Name));
                    curl_mime_name(Part, "data");
                }
            }

            Part = curl_mime_addpart(Mime_Mail);
            curl_mime_data(Part, TCHAR_TO_UTF8(*Message), CURL_ZERO_TERMINATED);
            curl_slist* slist = curl_slist_append(NULL, "Content-Disposition: inline");
            curl_mime_headers(Part, slist, 1);
            curl_easy_setopt(Curl, CURLOPT_MIMEPOST, Mime_Mail);

            /*
            * Send mail and free memories.
            */

            Response = curl_easy_perform(Curl);
            if (Response != CURLE_OK)
            {
                curl_slist_free_all(Recipients);
                curl_slist_free_all(slist);
                curl_easy_cleanup(Curl);

                AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                    {
                        DelegateMailSent.ExecuteIfBound(false, "Libcurl couldn't perform.");

                    }
                );

                return;
            }

            curl_slist_free_all(Recipients);
            curl_slist_free_all(slist);
            curl_easy_cleanup(Curl);

            AsyncTask(ENamedThreads::GameThread, [DelegateMailSent]()
                {
                    DelegateMailSent.ExecuteIfBound(true, "Mail successfully sent.");
                }
            );
        }
    );
}