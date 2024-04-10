// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "FF_HTTP_BasicBPLibrary.h"

// UE Server Includes.
#include "IHttpRouter.h"
#include "HttpResultCallback.h"

#include "FF_HTTP_Server.generated.h"

/*
Add these lines to Project/Config/DefaultEngine.ini
[HTTPServer.Listeners]
DefaultBindAddress = 0.0.0.0
*/

USTRUCT(BlueprintType)
struct FF_HTTP_BASIC_API FHeaderValues
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Array_String;

};

USTRUCT(BlueprintType)
struct FF_HTTP_BASIC_API FHttpServerMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> Body;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Query;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FHeaderValues> Headers;

	UPROPERTY(BlueprintReadOnly)
	FString Head;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	FString Method;

	UPROPERTY(BlueprintReadOnly)
	FString Proto;

	UPROPERTY(BlueprintReadOnly)
	FString RelativePath;

	/** UE Basic Server Only */
	UPROPERTY(BlueprintReadOnly)
	FString PeerAddress;

	/** UE Basic Server Only */
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Path_Params;

};

UCLASS(BlueprintType)
class FF_HTTP_BASIC_API UHttpServerBasicResponse : public UObject
{
	GENERATED_BODY()

public:

	FHttpResultCallback Response_Callback;

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|HTTP|Server|Basic")
	virtual void SendResponse(TMap<FString, FHeaderValues> In_Headers, FString In_Body, EHttpContentTypes ContentType = EHttpContentTypes::TEXT, EHttpResponseCodesBp ResponseCode = EHttpResponseCodesBp::Ok)
	{
		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(In_Body, UFF_HTTP_BasicBPLibrary::HTTP_Content_Types_To_String(ContentType));
		
		// Default Headers.
		Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
		Response->Headers.Add(TEXT("Access-Control-Allow-Methods"), { TEXT("GET,POST,PUT,PATCH,DELETE,OPTIONS") });

		// Additional Headers.
		TMap<FString, TArray<FString>> Response_Headers;
		
		for (TPair<FString, FHeaderValues>& Pair_Headers : In_Headers)
		{
			Response_Headers.Add(Pair_Headers.Key, Pair_Headers.Value.Array_String);
		}

		Response->Code = (EHttpServerResponseCodes)UFF_HTTP_BasicBPLibrary::HTTP_Convert_Response_Codes(ResponseCode);
		this->Response_Callback(MoveTemp(Response));

		return;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateHttpMessageBasic, UHttpServerBasicResponse*, Connection, FHttpServerMessage, Request);

UCLASS()
class FF_HTTP_BASIC_API AHTTP_Server_Basic : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TSharedPtr<IHttpRouter, ESPMode::ThreadSafe> httpRouter = nullptr;
	TArray<FHttpRouteHandle> Array_Route_Handles;

	virtual EHttpServerRequestVerbs BpRequestToUeRequest(EHttpRequestTypes BP_Request);

public:	

	// Sets default values for this actor's properties
	AHTTP_Server_Basic();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "", ExposeOnSpawn = "true"), Category = "Frozen Forest|HTTP|Server|Basic")
	int32 Port = 8082;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "", ExposeOnSpawn = "true"), Category = "Frozen Forest|HTTP|Server|Basic")
	TMap<FString, EHttpRequestTypes> Routes;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|HTTP Server|Basic")
	FDelegateHttpMessageBasic DelegateHttpMessageBasic;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "HTTP Server Basic - Start", Category = "Frozen Forest|HTTP|Server|Basic"))
	virtual void HttpServer_Basic_Start();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "HTTP Server Basic - Stop", Category = "Frozen Forest|HTTP|Server|Basic"))
	virtual void HttpServer_Basic_Stop();

};