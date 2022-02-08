FROM mcr.microsoft.com/dotnet/aspnet:5.0 AS base
WORKDIR /app
EXPOSE 80


FROM mcr.microsoft.com/dotnet/sdk:5.0 AS build
WORKDIR /src

COPY ["remote.csproj", "."]
RUN dotnet restore "remote.csproj"


# copy setting from setting prod folder to workdir
COPY . .
WORKDIR "/src"
RUN dotnet build "remote.csproj" -c Release -o /app/build




FROM build AS publish 
RUN dotnet publish "remote.csproj" -c Release -o /app/publish

ENV URL=https://host.thinkmay.net/Session/Setting

WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "remote.dll"]