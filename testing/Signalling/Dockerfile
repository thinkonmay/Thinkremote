FROM mcr.microsoft.com/dotnet/aspnet:5.0 AS base
WORKDIR /app
EXPOSE 80
EXPOSE 443





FROM mcr.microsoft.com/dotnet/sdk:5.0 AS buildprod
WORKDIR /src

COPY ["Signalling/Signalling.csproj", "Signalling/"]
COPY ["SharedHost/setting/prod/appsettings.json", "Signalling/"]
RUN dotnet restore "Signalling/Signalling.csproj"


# copy setting from setting prod folder to workdir
COPY . .
WORKDIR "/src/Signalling"
RUN dotnet build "Signalling.csproj" -c Release -o /app/build






FROM mcr.microsoft.com/dotnet/sdk:5.0 AS builddev
WORKDIR /src

COPY ["Signalling/Signalling.csproj", "Signalling/"]
COPY ["SharedHost/setting/dev/appsettings.json", "Signalling/"]
RUN dotnet restore "Signalling/Signalling.csproj"


# copy setting from setting dev folder to workdir
COPY . .
WORKDIR "/src/Signalling"
RUN dotnet build "Signalling.csproj" -c Release -o /app/build




FROM builddev AS publishdev
RUN dotnet publish "Signalling.csproj" -c Release -o /app/publish

FROM buildprod AS publishprod
RUN dotnet publish "Signalling.csproj" -c Release -o /app/publish






# production and development image both base on base image
FROM base AS dev
WORKDIR /app
COPY --from=publishdev /app/publish .
ENTRYPOINT ["dotnet", "Signalling.dll"]

FROM base AS prod
WORKDIR /app
COPY --from=publishprod /app/publish .
ENTRYPOINT ["dotnet", "Signalling.dll"]